// ───────────────────────────────────────────────────────────────────
// 文件: RtPipeline.cpp
// 用途: 管线编排器实现。串联配置加载、场景构建(批次2-4)、遗留批次5-9自检链、
//       A1真实生产链(搜索→EM→导出)及可选SBR覆盖通道。
// 所属模块: 应用层
// ───────────────────────────────────────────────────────────────────

#include "RtPipeline.h"

#include <fstream>

#include "RtRealChainRunner.h"
#include "../core/search/SbrEngine.h"
#include "../core/common/config/AppConfig.h"
#include "../core/common/config/AppConfigLoader.h"
#include "../core/common/config/AppConfigSnapshotWriter.h"
#include "../core/common/config/AppConfigValidator.h"
#include "../core/common/config/ConfigSelfCheck.h"
#include "../core/common/log/Logger.h"
#include "../core/common/material/MaterialDatabase.h"
#include "../core/common/version/VersionInfo.h"
#include "../core/search/SearchEngine.h"
#include "../core/path/PathSearchContext.h"
#include "../preprocess/build/SceneImporter.h"
#include "../preprocess/build/SceneTopologyBuilder.h"
#include "../preprocess/build/SceneQueryBuilder.h"
#include "../preprocess/accel/SceneVisibilityBuilder.h"  // v8: PVS precompute
#include "../core/search/PathReuseEngine.h"              // v8: path reuse
#include "../core/search/RxSeedSampler.h"                // v8: seed Rx selection
#include <set>                                           // v8: PVS 2-hop expansion

#include <sstream>

namespace rt {

namespace {

/// <summary>
/// Writes every warning and error from a ConfigValidationResult into the unified logger.
/// </summary>
/// <param name="logger">Initialized logger instance.</param>
/// <param name="validation">Config validation result to log.</param>
void LogValidationResult(Logger& logger, const ConfigValidationResult& validation)
{
    for (const std::string& warning : validation.warnings)
    {
        logger.Log(LogLevel::Warn, "Module1", warning);
    }

    for (const std::string& error : validation.errors)
    {
        logger.Log(LogLevel::Error, "Module1", error);
    }
}

/// <summary>
/// Constructs the minimal PathSearchContext for Batch-5 from config, scene, and material DB.
/// Copies TX/RX debug positions and wires the scene query pointer.
/// </summary>
/// <param name="config">Application configuration.</param>
/// <param name="scene">Static scene with geometry and query facade.</param>
/// <param name="matDb">Material database (dielectric constants).</param>
/// <returns>Populated PathSearchContext ready for the SearchEngine.</returns>
PathSearchContext BuildBatch5SearchContext(const AppConfig& config, const Scene& scene, const MaterialDatabase* matDb)
{
    PathSearchContext context;
    context.config = &config;
    context.scene = &scene;
    context.scene_query = scene.query.get();
    context.material_db = matDb;
    context.tx_point.x = config.path_search.tx_x;
    context.tx_point.y = config.path_search.tx_y;
    context.tx_point.z = config.path_search.tx_z;
    context.rx_point.x = config.path_search.rx_x;
    context.rx_point.y = config.path_search.rx_y;
    context.rx_point.z = config.path_search.rx_z;
    return context;
}

} // namespace

/// <summary>
/// Runs the full pipeline: config load/validation, scene batches 2-4, Batch-5 search,
/// A1 real chain, and optional SBR coverage sweep.
/// </summary>
/// <param name="configPath">Path to the JSON configuration file.</param>
/// <returns>Structured result with success flag, exit code, and completed batch number.</returns>
PipelineRunResult RtPipeline::Run(const std::string& configPath) const
{
    PipelineRunResult runResult;

    // --- Batch 0/1: Config loading, validation, and module-1 self-check ---
    const AppConfigLoadResult loadResult = LoadAppConfigFromJsonFile(configPath);

    Logger logger;
    logger.Initialize(loadResult.config.app_runtime);

    const VersionInfo versionInfo = VersionInfo::Current();
    logger.Log(LogLevel::Info, "App", "RT 流水线启动。");
    logger.Log(LogLevel::Info, "App", "配置文件: " + configPath);
    logger.Log(LogLevel::Info, "App", "程序版本: " + versionInfo.program_version);
    logger.Log(LogLevel::Info, "App", "配置架构版本: " + versionInfo.config_schema_version);
    logger.Log(LogLevel::Info, "App", "运行标识: " + loadResult.config.app_runtime.run_id);

    for (const RtError& error : loadResult.errors)
    {
        logger.LogError("Module1", error);
    }

    if (!loadResult.load_succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "配置加载失败，无法继续。");
        return runResult;
    }

    const ConfigValidationResult validation = ValidateAppConfig(loadResult.config);
    LogValidationResult(logger, validation);

    if (!validation.passed)
    {
        logger.Log(LogLevel::Fatal, "App", "配置校验不通过。");
        return runResult;
    }

    std::ostringstream summary;
    summary << "校验通过。模式=" << loadResult.config.app_runtime.mode
            << ", 日志级别=" << loadResult.config.app_runtime.log_level
            << ", 频率=" << loadResult.config.em_solver.frequency_hz << " Hz";
    logger.Log(LogLevel::Info, "App", summary.str());

    if (loadResult.config.output.export_config_snapshot)
    {
        const AppConfigSnapshotWriteResult snapshotWriteResult = WriteAppConfigSnapshot(loadResult.config);
        if (!snapshotWriteResult.succeeded)
        {
            logger.LogError("Module1", snapshotWriteResult.error);
            logger.Log(LogLevel::Fatal, "App", "配置快照导出失败。");
            return runResult;
        }

        logger.Log(LogLevel::Info, "App", "配置快照已导出: " + snapshotWriteResult.output_file_path);
    }

    if (loadResult.config.validation.run_module1_self_check) {
        const ConfigSelfCheckResult selfCheckResult = RunModule1SelfCheck(loadResult.config);
        for (const std::string& detail : selfCheckResult.details)
            logger.Log(LogLevel::Info, "模块1", "自检: " + detail);
        if (!selfCheckResult.succeeded) {
            logger.LogError("模块1", selfCheckResult.error);
            logger.Log(LogLevel::Fatal, "App", "模块1自检未通过。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "模块1", "模块1自检通过。");
    }

    logger.Log(LogLevel::Info, "App", "批次0/1: 启动闭环完成。");

    // Batch2: 场景导入与语义恢复
    const SceneBatch2BuildResult batch2Result = BuildSceneForBatch2(loadResult.config);
    for (const RtError& error : batch2Result.errors) logger.LogError("模块2", error);
    if (!batch2Result.succeeded) {
        logger.Log(LogLevel::Fatal, "App", "批次2: 场景导入与语义恢复失败。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "App", "批次2: 场景导入与语义恢复完成。");

    const SceneBatch3BuildResult batch3Result = BuildSceneForBatch3(loadResult.config, batch2Result.scene);
    for (const RtError& error : batch3Result.errors) logger.LogError("模块2", error);
    if (!batch3Result.succeeded) {
        logger.Log(LogLevel::Fatal, "App", "批次3: 拓扑、诊断与加速结构构建失败。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "App", "批次3: 拓扑、诊断与加速结构完成。");

    SceneBatch4BuildResult batch4Result = BuildSceneForBatch4(loadResult.config, batch3Result.scene);
    for (const RtError& error : batch4Result.errors) logger.LogError("模块2", error);
    if (!batch4Result.succeeded) {
        logger.Log(LogLevel::Fatal, "App", "批次4: 查询门面与场景缓存构建失败。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "App", "批次4: 查询门面与场景缓存完成。");

    // 材质数据库: 加载介电常数
    MaterialDatabase matDb;
    if (!loadResult.config.material.material_database_file.empty()) {
        matDb.LoadFromCsv(loadResult.config.material.material_database_file);
        logger.Log(LogLevel::Info, "App", "材质数据库已加载: " + loadResult.config.material.material_database_file);
    }

    // v7.3: 预查所有面元电参数 (后续SBR热路径O(1)读取)
    if (!matDb.empty()) {
        const double fq = loadResult.config.em_solver.frequency_hz;
        for (Face& face : batch4Result.scene.faces) {
            if (!face.surface_material_name.empty()) {
                MaterialProps sp = matDb.QueryByName(face.surface_material_name, fq);
                face.surface_eps_r = sp.epsilon_r;
                face.surface_sigma = sp.sigma;
            }
        }
    }

    // v8 Phase 1: 场景可见性预计算 (PVS + Edge Adjacency + Angular Grid)
    if (loadResult.config.pipeline.enable_stage0_precompute && batch4Result.scene.query) {
        SceneVisibilityBuilder::BuildAll(batch4Result.scene, *batch4Result.scene.query, loadResult.config);
        std::ostringstream pvsLog;
        pvsLog << "预计算完成: PVS=" << batch4Result.scene.visibility.face_pvs.total_entries
               << " entries, EdgeAdj=" << batch4Result.scene.visibility.edge_adjacency.total_edges
               << " pairs, AngularGrid=" << batch4Result.scene.visibility.angular_grid.CellCount()
               << " cells, time=" << batch4Result.scene.visibility.build_time_seconds << "s";
        logger.Log(LogLevel::Info, "S0", pvsLog.str());
    }

    // v8 Phase 2: SBR粗扫 + Rx种子采样 (收集活跃面元集)
    SbrCoarseResult coarseResult;
    if (loadResult.config.pipeline.enable_stage1_coarse_sbr && batch4Result.scene.query) {
        SbrEngine sbrEngine;
        SbrCoarseContext coarseCtx;
        coarseCtx.config = &loadResult.config;
        coarseCtx.scene = &batch4Result.scene;
        coarseCtx.scene_query = batch4Result.scene.query.get();
        coarseCtx.tx_point = MakeVec3(loadResult.config.path_search.tx_x,
                                       loadResult.config.path_search.tx_y,
                                       loadResult.config.path_search.tx_z);
        coarseCtx.rx_grid = {}; // 从path_search单Rx创建
        coarseCtx.rx_grid.push_back(MakeVec3(loadResult.config.path_search.rx_x,
                                              loadResult.config.path_search.rx_y,
                                              loadResult.config.path_search.rx_z));
        coarseCtx.coarse_ray_count = 200000;
        coarseCtx.coarse_max_depth = 5;
        coarseCtx.coarse_rx_sphere_radius = 2.0;
        coarseCtx.expand_pvs = batch4Result.scene.visibility.face_pvs.valid;
        coarseResult = sbrEngine.RunCoarsePass(coarseCtx);
        for (auto& line : coarseResult.trace_lines)
            logger.Log(LogLevel::Info, "S1", line);

        // RxSeedSampler
        RxSeedSamplerConfig seedCfg;
        seedCfg.target_seed_count = loadResult.config.pipeline.seed_rx_count;
        seedCfg.uniform_stride_m = loadResult.config.pipeline.seed_spatial_stride;
        auto seeds = RxSeedSampler::Sample(coarseCtx.rx_grid, coarseResult.rx_active_mask,
                                            coarseResult.rx_active_faces, seedCfg);
        std::ostringstream seedLog;
        seedLog << "种子Rx: " << seeds.seed_count << " seeds, avg "
                << seeds.avg_rx_per_seed << " Rx/seed, max " << seeds.max_rx_per_seed;
        logger.Log(LogLevel::Info, "S1", seedLog.str());
    }

    // --- Search context setup: build the minimal PathSearchContext for Batch 5 ---
    const PathSearchContext batch5SearchContext = BuildBatch5SearchContext(loadResult.config, batch4Result.scene, &matDb);
    SearchEngine searchEngine;
    SearchEngineResult batch5Result;

    // v8 hybrid: AngularGrid方向查询 + 2-hop PVS扩展 → 全量约束搜索
    if (coarseResult.succeeded && loadResult.config.pipeline.enable_stage2_constrained_search
        && batch4Result.scene.visibility.face_pvs.valid
        && batch4Result.scene.visibility.angular_grid.valid) {
        const auto& pvs = batch4Result.scene.visibility.face_pvs;
        const auto& ag = batch4Result.scene.visibility.angular_grid;

        for (auto& kv : coarseResult.rx_active_faces) {
            std::set<int> expanded;
            // 从AngularGrid收集: 对Tx位置的各方向查询可见面元
            Point3 txPt = MakeVec3(loadResult.config.path_search.tx_x,
                                    loadResult.config.path_search.tx_y,
                                    loadResult.config.path_search.tx_z);
            const int nAzi=ag.n_azimuth, nZen=ag.n_zenith;
            for (int azi=0; azi<nAzi; azi+=4) {    // 降采样加速
                for (int zen=0; zen<nZen; zen+=2) {
                    int ci = ag.CellIndex(azi, zen);
                    if (ci>=0 && ci<static_cast<int>(ag.cells.size())) {
                        for (int fid : ag.cells[ci]) expanded.insert(fid);
                    }
                }
            }
            // 2-hop PVS扩展
            for (int hop=0; hop<2; ++hop) {
                std::vector<int> cur(expanded.begin(), expanded.end());
                for (int fid : cur) if (pvs.HasEntry(fid))
                    for (int pf : pvs.GetVisibleFaces(fid)) expanded.insert(pf);
            }
            size_t sz0 = kv.second.size();
            kv.second.assign(expanded.begin(), expanded.end());
            std::sort(kv.second.begin(), kv.second.end());
            logger.Log(LogLevel::Info, "S1", "Rx"+std::to_string(kv.first)+": "
                +std::to_string(sz0)+"→"+std::to_string(kv.second.size())+" faces (AngularGrid+PVS2-hop)");
        }
    }
    bool usedConstrainedSearch = false;
    if (loadResult.config.pipeline.enable_stage2_constrained_search
        && coarseResult.succeeded && !coarseResult.rx_active_faces.empty())
    {
        int rxIdx = 0;
        ConstrainedSearchConfig csc;
        auto itFaces = coarseResult.rx_active_faces.find(rxIdx);
        auto itWedges = coarseResult.rx_active_wedges.find(rxIdx);
        if (itFaces != coarseResult.rx_active_faces.end() && !itFaces->second.empty()) {
            csc.candidate_faces = &itFaces->second;
            logger.Log(LogLevel::Info, "S2", "约束搜索: candidate_faces="
                + std::to_string(itFaces->second.size()) + " faces");
        }
        if (itWedges != coarseResult.rx_active_wedges.end() && !itWedges->second.empty()) {
            csc.candidate_wedges = &itWedges->second;
            logger.Log(LogLevel::Info, "S2", "约束搜索: candidate_wedges="
                + std::to_string(itWedges->second.size()) + " wedges");
        }
        batch5Result = searchEngine.Run(batch5SearchContext, csc);
        usedConstrainedSearch = true;
    }
    else
    {
        batch5Result = searchEngine.Run(batch5SearchContext);  // full search
    }

    // ── Phase 4: 路径复用验证 (有种子Rx时) ──
    if (loadResult.config.pipeline.enable_stage3_path_reuse
        && usedConstrainedSearch && batch5Result.succeeded
        && !batch5Result.path_set.paths.empty())
    {
        Point3 refRx = MakeVec3(loadResult.config.path_search.rx_x,
                                 loadResult.config.path_search.rx_y,
                                 loadResult.config.path_search.rx_z);
        Point3 neighborRx = MakeVec3(refRx.x + 0.5, refRx.y, refRx.z);  // 50cm offset
        PathReuseConfig reuseCfg;
        reuseCfg.max_reuse_distance_m = loadResult.config.pipeline.reuse_max_distance;
        reuseCfg.verify_last_hop = loadResult.config.pipeline.reuse_verify_last_hop;
        PathReuseResult reuseResult = PathReuseEngine::ReusePaths(
            refRx, batch5Result.path_set.paths, neighborRx,
            *batch4Result.scene.query, loadResult.config, reuseCfg);
        std::ostringstream reuseLog;
        reuseLog << "路径复用: " << reuseResult.reused_count << " reused, "
                 << reuseResult.rejected_visibility << " rejected (visibility), "
                 << reuseResult.rejected_distance << " rejected (distance)";
        logger.Log(LogLevel::Info, "S3", reuseLog.str());
    }

    if (usedConstrainedSearch) {
        logger.Log(LogLevel::Info, "S2", "约束搜索完成: " + std::to_string(batch5Result.path_set.paths.size()) + " paths");
    }

    if (!batch5Result.succeeded || batch5Result.path_set.paths.empty()) {
        logger.Log(LogLevel::Fatal, "App", "搜索器未能建立基本LOS闭环。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "App", "批次5~9: 搜索/扩展器/EM/汇总/导出 全部完成。");

    // --- A1 chain execution: the real production Search->EM->Export pipeline ---
    const bool isCoverageOnly = loadResult.config.sbr.enabled
        && loadResult.config.em_solver.solver_mode == "Coverage";

    A1RealChainRunResult a1Result;
    if (!isCoverageOnly) {
        a1Result = RunA1RealChain(loadResult.config, batch4Result.scene, batch5Result, logger, &matDb);
        if (!a1Result.succeeded)
        {
            logger.Log(LogLevel::Fatal, "App", "A1真实生产链执行失败。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "App", "A1真实生产链闭环完成。");
    } else {
        logger.Log(LogLevel::Info, "App", "Coverage-only: 跳过precise IM搜索, 直接进入SBR");
    }

    // SBR覆盖仿真(可选)
    if (loadResult.config.sbr.enabled) {
        logger.Log(LogLevel::Info, "App", "SBR覆盖模式已启用，构建Rx网格...");

        SbrContext sbrCtx;
        sbrCtx.config = &loadResult.config;
        sbrCtx.scene = &batch4Result.scene;
        sbrCtx.scene_query = batch4Result.scene.query.get();
        sbrCtx.tx_point.x = loadResult.config.path_search.tx_x;
        sbrCtx.tx_point.y = loadResult.config.path_search.tx_y;
        sbrCtx.tx_point.z = loadResult.config.path_search.tx_z;

        const auto& sc = loadResult.config.sbr;
        double gxMin = sc.rx_grid_min_x, gxMax = sc.rx_grid_max_x;
        double gyMin = sc.rx_grid_min_y, gyMax = sc.rx_grid_max_y;
        double gzMin = sc.rx_grid_min_z, gzMax = sc.rx_grid_max_z;

        // 自动从场景AABB推导网格范围
        if (sc.auto_grid_bounds && batch4Result.scene.acceleration.face_acceleration.valid) {
            const auto& sb = batch4Result.scene.acceleration.face_acceleration.scene_bounds;
            if (sb.valid) {
                double m = std::max({sc.rx_grid_step_x, sc.rx_grid_step_y, sc.rx_grid_step_z}); // v6: auto
                gxMin = sb.min.x + m; gxMax = sb.max.x - m;
                gyMin = sb.min.y + m; gyMax = sb.max.y - m;
                gzMin = sb.min.z + m; gzMax = sb.max.z - m;
            }
        }

        for (double rx = gxMin; rx <= gxMax + 1e-9; rx += sc.rx_grid_step_x)
            for (double ry = gyMin; ry <= gyMax + 1e-9; ry += sc.rx_grid_step_y)
                for (double rz = gzMin; rz <= gzMax + 1e-9; rz += sc.rx_grid_step_z)
                    sbrCtx.rx_grid.push_back(MakeVec3(rx, ry, rz));
        sbrCtx.store_paths = sc.store_paths;
        sbrCtx.tx_power_dBm = sc.tx_power_dBm;
        sbrCtx.material_db = &matDb;

        SbrEngine sbrEngine;
        SbrCoverageResult sbrResult = sbrEngine.Run(sbrCtx);

        for (const auto& line : sbrResult.trace_lines)
            logger.Log(LogLevel::Info, "SBR", line);

        std::ostringstream sbrSum;
        sbrSum << "SBR coverage completed: rays=" << sbrResult.total_rays
               << ", activeRx=" << sbrResult.active_rx_count
               << "/" << sbrCtx.rx_grid.size();
        logger.Log(LogLevel::Info, "SBR", sbrSum.str());

        // 导出SBR覆盖结果JSON (v5 D3: 每Rx功率+路径+命中数)
        std::string sbrOutDir = "output/" + loadResult.config.app_runtime.run_id + "/coverage";
        std::string sbrJsonPath = sbrOutDir + "/sbr_coverage.json";
        std::ofstream sbrFile(sbrJsonPath);
        if (sbrFile.is_open()) {
            sbrFile << "{\n";
            sbrFile << "  \"total_rays\": " << sbrResult.total_rays << ",\n";
            sbrFile << "  \"active_rx_count\": " << sbrResult.active_rx_count << ",\n";
            sbrFile << "  \"rx_grid_count\": " << sbrCtx.rx_grid.size() << ",\n";
            sbrFile << "  \"tx_power_dBm\": " << loadResult.config.sbr.tx_power_dBm << ",\n";
            sbrFile << "  \"rx_sphere_radius_m\": " << loadResult.config.sbr.rx_sphere_radius_m << ",\n";
            sbrFile << "  \"records\": [\n";
            for (size_t i = 0; i < sbrResult.rx_records.size(); ++i) {
                const auto& rec = sbrResult.rx_records[i];
                sbrFile << "    {";
                sbrFile << "\"rx_index\": " << rec.rx_index << ", ";
                sbrFile << "\"x\": " << rec.rx_position.x << ", ";
                sbrFile << "\"y\": " << rec.rx_position.y << ", ";
                sbrFile << "\"z\": " << rec.rx_position.z << ", ";
                sbrFile << "\"power_dBm\": " << rec.total_power_dBm << ", ";
                sbrFile << "\"power_linear\": " << rec.total_power_linear << ", ";
                sbrFile << "\"ray_hit_count\": " << rec.ray_hit_count << ", ";
                sbrFile << "\"path_count\": " << rec.paths.size();
                sbrFile << "}";
                if (i < sbrResult.rx_records.size() - 1) sbrFile << ",";
                sbrFile << "\n";
            }
            sbrFile << "  ]\n";
            sbrFile << "}\n";
            sbrFile.close();
            logger.Log(LogLevel::Info, "SBR", "SBR coverage exported: " + sbrJsonPath);
        }

        // v8: SBR→Precise EM — 将SBR记录的几何路径送入精确电场求解器
        if (sbrResult.succeeded && sbrCtx.store_paths && sbrResult.active_rx_count > 0) {
            int totalPaths=0, validEM=0;
            for (auto& rec : sbrResult.rx_records) {
                if (rec.paths.empty()) continue;
                totalPaths += static_cast<int>(rec.paths.size());
                double complexSumRe=0.0, complexSumIm=0.0;
                for (GeometricPath& gp : rec.paths) {
                    if (!gp.valid) continue;
                    EMPathResult em;
                    if (SolveSinglePathEM(loadResult.config, batch4Result.scene, gp, em, &matDb)) {
                        complexSumRe += em.amplitude_real;
                        complexSumIm += em.amplitude_imag;
                        validEM++;
                    }
                }
                // 相干叠加功率覆盖原有标量功率
                rec.total_power_linear = complexSumRe*complexSumRe + complexSumIm*complexSumIm;
                rec.total_power_dBm = 10.0 * std::log10(std::max(1e-30, rec.total_power_linear * std::pow(10.0, loadResult.config.sbr.tx_power_dBm/10.0)));
            }
            std::ostringstream emLog;
            emLog << "SBR→Precise EM: " << validEM << "/" << totalPaths
                  << " paths solved, " << sbrResult.active_rx_count << " Rx coherent-summed";
            logger.Log(LogLevel::Info, "SBR-EM", emLog.str());
        }
    }

    runResult.succeeded = true;
    runResult.exit_code = 0;
    runResult.completed_batch = 10;
    return runResult;
}

} // namespace rt
