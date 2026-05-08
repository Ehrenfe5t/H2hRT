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

    const SceneBatch4BuildResult batch4Result = BuildSceneForBatch4(loadResult.config, batch3Result.scene);
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

    // --- Search context setup: build the minimal PathSearchContext for Batch 5 ---
    const PathSearchContext batch5SearchContext = BuildBatch5SearchContext(loadResult.config, batch4Result.scene, &matDb);
    SearchEngine searchEngine;
    const SearchEngineResult batch5Result = searchEngine.Run(batch5SearchContext);

    if (!batch5Result.succeeded || batch5Result.path_set.paths.empty()) {
        logger.Log(LogLevel::Fatal, "App", "搜索器未能建立基本LOS闭环。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "App", "批次5~9: 搜索/扩展器/EM/汇总/导出 全部完成。");

    // --- A1 chain execution: the real production Search->EM->Export pipeline ---
    const A1RealChainRunResult a1Result = RunA1RealChain(loadResult.config, batch4Result.scene, batch5Result, logger, &matDb);
    if (!a1Result.succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "A1真实生产链执行失败。");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "A1真实生产链闭环完成。");

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
    }

    runResult.succeeded = true;
    runResult.exit_code = 0;
    runResult.completed_batch = 10;
    return runResult;
}

} // namespace rt
