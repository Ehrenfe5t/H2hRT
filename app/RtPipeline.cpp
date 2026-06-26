// RtPipeline.cpp
// v11 P2P 主链编排器：配置 -> 场景/材质 -> SceneQuery -> SBR P2P -> EM -> 导出。

#include "RtPipeline.h"

#include <fstream>
#include <filesystem>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <map>
#include <set>

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
#include "../core/search/SearchResult.h"
#include "../preprocess/build/SceneImporter.h"
#include "../preprocess/build/SceneTopologyBuilder.h"
#include "../preprocess/build/SceneQueryBuilder.h"
#include "../preprocess/accel/SceneVisibilityBuilder.h"

#include <sstream>

namespace rt {

namespace {

const char* InteractionTypeLabel(InteractionType type)
{
    switch (type)
    {
    case InteractionType::None: return "None";
    case InteractionType::Tx: return "Tx";
    case InteractionType::Rx: return "Rx";
    case InteractionType::Los: return "Los";
    case InteractionType::Reflection: return "Reflection";
    case InteractionType::Transmission: return "Transmission";
    case InteractionType::Diffraction: return "Diffraction";
    case InteractionType::Scattering: return "Scattering";
    default: return "Unknown";
    }
}

std::string JsonEscape(const std::string& value)
{
    std::ostringstream out;
    for (char c : value)
    {
        switch (c)
        {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: out << c; break;
        }
    }
    return out.str();
}

std::string PathSignatureHex(uint64_t signature)
{
    std::ostringstream out;
    out << "0x" << std::hex << signature;
    return out.str();
}

using RtClock = std::chrono::steady_clock;

double ElapsedSeconds(RtClock::time_point start, RtClock::time_point end)
{
    return std::chrono::duration<double>(end - start).count();
}

std::string FormatSeconds(double seconds)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(seconds < 10.0 ? 3 : 2) << seconds << " s";
    return out.str();
}

std::string FormatFrequencyGHz(double frequencyHz)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(3) << (frequencyHz / 1.0e9) << " GHz";
    return out.str();
}

void NormalizeRuntimeOutputPaths(AppConfig& config)
{
    const std::string runRoot = "output/" + config.app_runtime.run_id;
    config.app_runtime.log_file_path = runRoot + "/logs/rt.log";
    config.app_runtime.config_snapshot_directory = runRoot + "/config";
    config.app_runtime.cache_directory = runRoot + "/cache";
}

std::string InteractionShortName(InteractionType type)
{
    switch (type)
    {
    case InteractionType::Tx: return "Tx";
    case InteractionType::Rx: return "Rx";
    case InteractionType::Los: return "LOS";
    case InteractionType::Reflection: return "R";
    case InteractionType::Transmission: return "T";
    case InteractionType::Diffraction: return "D";
    case InteractionType::Scattering: return "S";
    default: return "N";
    }
}

std::string BuildPathSequenceLabel(const GeometricPath& path)
{
    std::ostringstream out;
    bool first = true;
    for (const PathNode& node : path.nodes)
    {
        if (!first) out << "-";
        first = false;
        out << InteractionShortName(node.interaction_type);
    }
    return out.str();
}

struct PathSummaryStats {
    int los_paths = 0;
    int reflection_paths = 0;
    int transmission_paths = 0;
    int diffraction_paths = 0;
    long long reflection_nodes = 0;
    long long transmission_nodes = 0;
    long long diffraction_nodes = 0;
    std::vector<std::pair<std::string, int>> top_sequences;
};

PathSummaryStats BuildPathSummaryStats(const std::vector<GeometricPath>& paths)
{
    PathSummaryStats stats;
    std::map<std::string, int> sequenceCounts;
    for (const GeometricPath& path : paths)
    {
        bool hasReflection = false;
        bool hasTransmission = false;
        bool hasDiffraction = false;
        for (const PathNode& node : path.nodes)
        {
            if (node.interaction_type == InteractionType::Reflection) {
                hasReflection = true;
                ++stats.reflection_nodes;
            } else if (node.interaction_type == InteractionType::Transmission) {
                hasTransmission = true;
                ++stats.transmission_nodes;
            } else if (node.interaction_type == InteractionType::Diffraction) {
                hasDiffraction = true;
                ++stats.diffraction_nodes;
            }
        }
        if (path.is_los) ++stats.los_paths;
        if (hasReflection) ++stats.reflection_paths;
        if (hasTransmission) ++stats.transmission_paths;
        if (hasDiffraction) ++stats.diffraction_paths;
        ++sequenceCounts[BuildPathSequenceLabel(path)];
    }

    stats.top_sequences.assign(sequenceCounts.begin(), sequenceCounts.end());
    std::sort(stats.top_sequences.begin(), stats.top_sequences.end(),
        [](const auto& a, const auto& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });
    if (stats.top_sequences.size() > 5) stats.top_sequences.resize(5);
    return stats;
}

std::string FormatTopSequences(const std::vector<std::pair<std::string, int>>& topSequences)
{
    if (topSequences.empty()) return "无";
    std::ostringstream out;
    for (std::size_t i = 0; i < topSequences.size(); ++i)
    {
        if (i > 0) out << "；";
        out << topSequences[i].first << ": " << topSequences[i].second;
    }
    return out.str();
}

void LogSbrKeyParameters(Logger& logger, const AppConfig& config)
{
    std::ostringstream out;
    out << "SBR 参数：射线数 " << config.sbr.ray_count
        << "，最大深度 " << config.sbr.max_ray_depth
        << "，R/T/D=" << config.sbr.max_reflection_count
        << "/" << config.sbr.max_transmission_count
        << "/" << config.sbr.max_diffraction_count
        << "，接收球半径 " << config.sbr.rx_sphere_radius_m << " m"
        << "，功率阈值 " << config.sbr.ray_power_threshold_dB << " dB";
    logger.Log(LogLevel::Info, "寻径", out.str());
}

std::string BuildTaskListText(const std::vector<P2PLinkTask>& tasks)
{
    std::ostringstream out;
    for (std::size_t i = 0; i < tasks.size(); ++i)
    {
        if (i > 0) out << "，";
        out << tasks[i].id;
    }
    return out.str();
}

bool IsNonZeroVector(double x, double y, double z)
{
    return std::fabs(x) > 1.0e-12 || std::fabs(y) > 1.0e-12 || std::fabs(z) > 1.0e-12;
}

AntennaConfig BuildTxAntennaConfigForTask(const TxTarget& tx)
{
    AntennaConfig antenna;
    antenna.source_type = tx.tx_source_type.empty() ? "Ideal" : tx.tx_source_type;
    antenna.pattern_file = tx.tx_pattern_file;
    antenna.polarization_file = tx.tx_polarization_file;
    if (IsNonZeroVector(tx.tx_forward_x, tx.tx_forward_y, tx.tx_forward_z)) {
        antenna.forward_x = tx.tx_forward_x;
        antenna.forward_y = tx.tx_forward_y;
        antenna.forward_z = tx.tx_forward_z;
    }
    if (IsNonZeroVector(tx.tx_up_x, tx.tx_up_y, tx.tx_up_z)) {
        antenna.up_x = tx.tx_up_x;
        antenna.up_y = tx.tx_up_y;
        antenna.up_z = tx.tx_up_z;
    }
    return antenna;
}

AntennaConfig BuildRxAntennaConfigForTask(const RxTarget& rx)
{
    AntennaConfig antenna;
    antenna.source_type = rx.rx_source_type.empty() ? "Ideal" : rx.rx_source_type;
    antenna.pattern_file = rx.rx_pattern_file;
    antenna.polarization_file = rx.rx_polarization_file;
    if (IsNonZeroVector(rx.rx_forward_x, rx.rx_forward_y, rx.rx_forward_z)) {
        antenna.forward_x = rx.rx_forward_x;
        antenna.forward_y = rx.rx_forward_y;
        antenna.forward_z = rx.rx_forward_z;
    }
    if (IsNonZeroVector(rx.rx_up_x, rx.rx_up_y, rx.rx_up_z)) {
        antenna.up_x = rx.rx_up_x;
        antenna.up_y = rx.rx_up_y;
        antenna.up_z = rx.rx_up_z;
    }
    return antenna;
}

AppConfig BuildConfigForP2PTask(const AppConfig& baseConfig,
                                const P2PLinkTask& task,
                                const std::string& baseRunId)
{
    AppConfig config = baseConfig;
    config.app_runtime.run_id = baseRunId + "/" + task.id;
    config.path_search.tx_x = task.tx.x;
    config.path_search.tx_y = task.tx.y;
    config.path_search.tx_z = task.tx.z;
    config.path_search.rx_x = task.rx.x;
    config.path_search.rx_y = task.rx.y;
    config.path_search.rx_z = task.rx.z;
    config.path_search.rx_list.clear();
    config.sbr.tx_power_dBm = task.tx.power_dBm;
    config.tx_antenna = BuildTxAntennaConfigForTask(task.tx);
    config.rx_antenna = BuildRxAntennaConfigForTask(task.rx);
    return config;
}

bool ExportSbrGeometryPaths(
    const std::string& runId,
    const SbrCoverageResult& result,
    const SbrContext& context,
    Logger& logger)
{
    const std::string root = "output/" + runId + "/paths";
    std::error_code ec;
    std::filesystem::create_directories(root, ec);
    if (ec)
    {
        logger.Log(LogLevel::Error, "导出", "几何路径导出目录创建失败：" + root);
        return false;
    }

    int fileCount = 0;
    long long exportedPathCount = 0;
    std::vector<std::string> files;

    for (const RxCoverageRecord& rec : result.rx_records)
    {
        if (rec.paths.empty())
        {
            continue;
        }

        const std::string fileName = "rx_" + std::to_string(rec.rx_index) + "_sbr_paths.json";
        const std::string filePath = root + "/" + fileName;
        std::ofstream out(filePath);
        if (!out.is_open())
        {
            logger.Log(LogLevel::Error, "导出", "几何路径文件打开失败：" + filePath);
            return false;
        }

        out << "{\n"
            << "  \"source\": \"sbr_v11_p2p_geometry_paths\",\n"
            << "  \"trace_profile\": \"" << JsonEscape(result.trace_profile) << "\",\n"
            << "  \"rx_index\": " << rec.rx_index << ",\n"
            << "  \"tx_position\": [" << context.tx_point.x << ", " << context.tx_point.y << ", " << context.tx_point.z << "],\n"
            << "  \"rx_position\": [" << rec.rx_position.x << ", " << rec.rx_position.y << ", " << rec.rx_position.z << "],\n"
            << "  \"path_count\": " << rec.paths.size() << ",\n"
            << "  \"paths\": [\n";

        for (std::size_t pi = 0; pi < rec.paths.size(); ++pi)
        {
            const GeometricPath& path = rec.paths[pi];
            out << "    {\n"
                << "      \"path_id\": " << path.path_id << ",\n"
                << "      \"path_signature\": \"" << PathSignatureHex(path.path_signature) << "\",\n"
                << "      \"valid\": " << (path.valid ? "true" : "false") << ",\n"
                << "      \"is_los\": " << (path.is_los ? "true" : "false") << ",\n"
                << "      \"contains_transmission\": " << (path.contains_transmission ? "true" : "false") << ",\n"
                << "      \"total_length_m\": " << path.total_length << ",\n"
                << "      \"geometry_residual\": " << path.geometry_residual << ",\n"
                << "      \"reflection_residual_m\": " << path.reflection_residual_m << ",\n"
                << "      \"max_snell_residual\": " << path.max_snell_residual << ",\n"
                << "      \"max_keller_residual\": " << path.max_keller_residual << ",\n"
                << "      \"residual_reject_reason\": \"" << JsonEscape(path.residual_reject_reason) << "\",\n"
                << "      \"nodes\": [\n";

            for (std::size_t ni = 0; ni < path.nodes.size(); ++ni)
            {
                const PathNode& node = path.nodes[ni];
                out << "        {"
                    << "\"interaction_type\": " << static_cast<int>(node.interaction_type)
                    << ", \"interaction_name\": \"" << InteractionTypeLabel(node.interaction_type) << "\""
                    << ", \"object_id\": " << node.object_id
                    << ", \"face_id\": " << node.face_id
                    << ", \"wedge_id\": " << node.wedge_id
                    << ", \"x\": " << node.point.x
                    << ", \"y\": " << node.point.y
                    << ", \"z\": " << node.point.z
                    << ", \"segment_length_m\": " << node.segment_length_from_previous
                    << ", \"incident_direction\": [" << node.incident_direction.x << ", " << node.incident_direction.y << ", " << node.incident_direction.z << "]"
                    << ", \"outgoing_direction\": [" << node.direction.x << ", " << node.direction.y << ", " << node.direction.z << "]"
                    << ", \"surface_normal\": [" << node.surface_normal.x << ", " << node.surface_normal.y << ", " << node.surface_normal.z << "]"
                    << ", \"snell_residual\": " << node.snell_residual
                    << ", \"keller_residual\": " << node.diffraction_diag.keller_residual
                    << "}";
                if (ni + 1U < path.nodes.size())
                {
                    out << ",";
                }
                out << "\n";
            }

            out << "      ]\n"
                << "    }";
            if (pi + 1U < rec.paths.size())
            {
                out << ",";
            }
            out << "\n";
        }

        out << "  ]\n"
            << "}\n";
        out.close();

        files.push_back(fileName);
        ++fileCount;
        exportedPathCount += static_cast<long long>(rec.paths.size());
    }

    const std::string manifestPath = root + "/sbr_paths_manifest.json";
    std::ofstream manifest(manifestPath);
    if (!manifest.is_open())
    {
        logger.Log(LogLevel::Error, "导出", "几何路径清单导出失败：" + manifestPath);
        return false;
    }
    manifest << "{\n"
             << "  \"source\": \"sbr_v11_p2p_geometry_paths\",\n"
             << "  \"trace_profile\": \"" << JsonEscape(result.trace_profile) << "\",\n"
             << "  \"rx_file_count\": " << fileCount << ",\n"
             << "  \"path_count\": " << exportedPathCount << ",\n"
             << "  \"files\": [";
    for (std::size_t i = 0; i < files.size(); ++i)
    {
        if (i > 0)
        {
            manifest << ", ";
        }
        manifest << "\"" << JsonEscape(files[i]) << "\"";
    }
    manifest << "]\n}\n";
    manifest.close();

    logger.Log(LogLevel::Info, "导出",
        "几何路径已导出：" + root + "（" +
        std::to_string(exportedPathCount) + " 条）");
    return true;
}

SearchEngineResult BuildSearchResultFromSbrRecord(
    const SbrCoverageResult& sbrResult,
    const RxCoverageRecord& record)
{
    SearchEngineResult result;
    result.succeeded = !record.paths.empty();
    result.source_tag = "sbr_v11_p2p_geometry_output";
    result.uses_real_scene_query = true;
    result.path_set.paths = record.paths;
    result.generated_state_count = static_cast<int>(sbrResult.rx_paths_recorded);
    result.candidate_state_count = static_cast<int>(sbrResult.paths_after_postprocess);
    result.accepted_state_count = static_cast<int>(result.path_set.paths.size());
    result.deduplicated_path_count = static_cast<int>(
        sbrResult.rx_paths_deduplicated +
        sbrResult.paths_pruned_by_post_dedup +
        sbrResult.paths_pruned_by_similarity +
        sbrResult.paths_pruned_by_top_n +
        sbrResult.paths_pruned_by_residual);
    result.trace_lines = sbrResult.trace_lines;
    return result;
}

SearchEngineResult RunSbrPointToPointSearch(
    const AppConfig& config,
    const Scene& scene,
    const MaterialDatabase* matDb,
    const Point3& rxPoint,
    Logger& logger,
    const std::string& taskLabel)
{
    SbrContext sbrCtx;
    sbrCtx.config = &config;
    sbrCtx.scene = &scene;
    sbrCtx.scene_query = scene.query.get();
    sbrCtx.material_db = matDb;
    sbrCtx.tx_point = MakeVec3(config.path_search.tx_x,
                               config.path_search.tx_y,
                               config.path_search.tx_z);
    sbrCtx.rx_grid.push_back(rxPoint);
    sbrCtx.store_paths = true;
    sbrCtx.tx_power_dBm = config.sbr.tx_power_dBm;

    SbrEngine sbrEngine;
    const RtClock::time_point searchStart = RtClock::now();
    SbrCoverageResult sbrResult = sbrEngine.RunPointToPoint(sbrCtx);
    const double searchSeconds = ElapsedSeconds(searchStart, RtClock::now());

    if (sbrResult.succeeded)
    {
        ExportSbrGeometryPaths(config.app_runtime.run_id, sbrResult, sbrCtx, logger);
    }

    if (!sbrResult.succeeded || sbrResult.rx_records.empty())
    {
        SearchEngineResult failed;
        failed.source_tag = "sbr_v11_p2p_geometry_output";
        failed.trace_lines = sbrResult.trace_lines;
        return failed;
    }

    SearchEngineResult result = BuildSearchResultFromSbrRecord(sbrResult, sbrResult.rx_records.front());
    const PathSummaryStats pathStats = BuildPathSummaryStats(result.path_set.paths);

    std::ostringstream summary;
    summary << taskLabel << " 寻径完成：原始路径 " << sbrResult.rx_paths_recorded
            << "，严格去重后 " << sbrResult.paths_after_postprocess
            << "，EM 可用路径 " << result.path_set.paths.size()
            << "，耗时 " << FormatSeconds(searchSeconds);
    logger.Log(result.succeeded ? LogLevel::Info : LogLevel::Error, "寻径", summary.str());

    std::ostringstream typeSummary;
    typeSummary << taskLabel << " 路径类型：LOS " << pathStats.los_paths
                << "，含反射 " << pathStats.reflection_paths
                << "，含透射 " << pathStats.transmission_paths
                << "，含绕射 " << pathStats.diffraction_paths;
    logger.Log(LogLevel::Info, "寻径", typeSummary.str());

    std::ostringstream nodeSummary;
    nodeSummary << taskLabel << " 交互节点：反射节点 " << pathStats.reflection_nodes
                << "，透射节点 " << pathStats.transmission_nodes
                << "，绕射节点 " << pathStats.diffraction_nodes;
    logger.Log(LogLevel::Info, "寻径", nodeSummary.str());

    logger.Log(LogLevel::Info, "寻径",
        taskLabel + " 主要交互序列：" + FormatTopSequences(pathStats.top_sequences));
    return result;
}

/// <summary>
/// Writes every warning and error from a ConfigValidationResult into the unified logger.
/// </summary>
/// <param name="logger">Initialized logger instance.</param>
/// <param name="validation">Config validation result to log.</param>
void LogValidationResult(Logger& logger, const ConfigValidationResult& validation)
{
    for (const std::string& warning : validation.warnings)
    {
        logger.Log(LogLevel::Warn, "配置", warning);
    }

    for (const std::string& error : validation.errors)
    {
        logger.Log(LogLevel::Error, "配置", error);
    }
}

} // namespace

/// <summary>
/// Runs the v11 main pipeline: config load/validation, scene import/preprocess/query,
/// SBR P2P geometry search, EM calculation, and result export.
/// </summary>
/// <param name="configPath">Path to the JSON configuration file.</param>
/// <returns>Structured result with success flag, exit code, and completed batch number.</returns>
PipelineRunResult RtPipeline::Run(const std::string& configPath) const
{
    PipelineRunResult runResult;

    const RtClock::time_point pipelineStart = RtClock::now();
    const RtClock::time_point configLoadStart = RtClock::now();
    const AppConfigLoadResult loadResult = LoadAppConfigFromJsonFile(configPath);
    AppConfig config = loadResult.config;
    NormalizeRuntimeOutputPaths(config);

    Logger logger;
    logger.Initialize(config.app_runtime);

    const VersionInfo versionInfo = VersionInfo::Current();
    static_cast<void>(versionInfo);
    logger.Log(LogLevel::Info, "运行时", "开始仿真：" + config.app_runtime.run_id);
    logger.Log(LogLevel::Info, "配置", "配置文件：" + configPath);
    logger.Log(LogLevel::Info, "配置", "日志文件：" + config.app_runtime.log_file_path);

    for (const RtError& error : loadResult.errors) {
        logger.LogError("配置", error);
    }
    if (!loadResult.load_succeeded) {
        logger.Log(LogLevel::Fatal, "配置", "配置读取失败。");
        return runResult;
    }

    const ConfigValidationResult validation = ValidateAppConfig(config);
    LogValidationResult(logger, validation);
    if (!validation.passed) {
        logger.Log(LogLevel::Fatal, "配置", "配置检查失败。");
        return runResult;
    }

    logger.Log(LogLevel::Info, "配置",
        "配置检查通过，耗时 " + FormatSeconds(ElapsedSeconds(configLoadStart, RtClock::now())) + "。");

    if (config.output.export_config_snapshot) {
        const AppConfigSnapshotWriteResult snapshotWriteResult = WriteAppConfigSnapshot(config);
        if (!snapshotWriteResult.succeeded) {
            logger.LogError("配置", snapshotWriteResult.error);
            logger.Log(LogLevel::Fatal, "配置", "配置快照导出失败。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "配置", "配置快照：" + snapshotWriteResult.output_file_path);
    }

    if (config.validation.run_module1_self_check) {
        const ConfigSelfCheckResult selfCheckResult = RunModule1SelfCheck(config);
        for (const std::string& detail : selfCheckResult.details) {
            logger.Log(LogLevel::Info, "配置", "自检：" + detail);
        }
        if (!selfCheckResult.succeeded) {
            logger.LogError("配置", selfCheckResult.error);
            logger.Log(LogLevel::Fatal, "配置", "配置自检失败。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "配置", "配置自检通过。");
    }

    const RtClock::time_point sceneImportStart = RtClock::now();
    const SceneBatch2BuildResult batch2Result = BuildSceneForBatch2(config);
    for (const RtError& error : batch2Result.errors) logger.LogError("场景", error);
    if (!batch2Result.succeeded) {
        logger.Log(LogLevel::Fatal, "场景", "场景导入或材质绑定失败。");
        return runResult;
    }
    {
        std::ostringstream out;
        out << "场景导入完成：顶点 " << batch2Result.scene.vertices.size()
            << "，面元 " << batch2Result.scene.faces.size()
            << "，物体 " << batch2Result.scene.objects.size()
            << "，材质绑定 " << batch2Result.scene.material_bindings.size()
            << "，耗时 " << FormatSeconds(ElapsedSeconds(sceneImportStart, RtClock::now())) << "。";
        logger.Log(LogLevel::Info, "场景", out.str());
    }

    const RtClock::time_point preprocessStart = RtClock::now();
    const SceneBatch3BuildResult batch3Result = BuildSceneForBatch3(config, batch2Result.scene);
    for (const RtError& error : batch3Result.errors) logger.LogError("预处理", error);
    if (!batch3Result.succeeded) {
        logger.Log(LogLevel::Fatal, "预处理", "场景拓扑或预处理失败。");
        return runResult;
    }
    {
        const auto& acc = batch3Result.scene.acceleration;
        std::ostringstream out;
        out << "场景预处理完成：边 " << batch3Result.scene.edges.size()
            << "，楔边 " << batch3Result.scene.wedges.size()
            << "，FaceBVH 节点 " << acc.face_acceleration.bvh_node_count
            << "，叶节点 " << acc.face_acceleration.leaf_node_count
            << "，最大深度 " << acc.diagnostics.face_bvh_max_depth
            << "，BVH 构建耗时 " << acc.diagnostics.face_bvh_build_time_ms << " ms"
            << "，总耗时 " << FormatSeconds(ElapsedSeconds(preprocessStart, RtClock::now())) << "。";
        logger.Log(LogLevel::Info, "预处理", out.str());
        if (config.sbr.max_diffraction_count > 0) {
            logger.Log(LogLevel::Info, "预处理",
                "绕射棱边构建完成：满足 UTD 条件棱边 " +
                std::to_string(acc.wedge_acceleration.diffractable_wedge_count) + "。");
        }
    }

    const RtClock::time_point queryStart = RtClock::now();
    SceneBatch4BuildResult batch4Result = BuildSceneForBatch4(config, batch3Result.scene);
    for (const RtError& error : batch4Result.errors) logger.LogError("预处理", error);
    if (!batch4Result.succeeded) {
        logger.Log(LogLevel::Fatal, "预处理", "SceneQuery 构建失败。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "预处理",
        "SceneQuery 构建完成，耗时 " + FormatSeconds(ElapsedSeconds(queryStart, RtClock::now())) + "。");

    MaterialDatabase matDb;
    if (!config.material.material_database_file.empty()) {
        const bool loaded = matDb.LoadFromCsv(config.material.material_database_file);
        if (!loaded) {
            logger.Log(LogLevel::Error, "材料",
                "材料数据库读取失败：" + config.material.material_database_file);
            runResult.succeeded = false;
            runResult.exit_code = static_cast<int>(ErrorCode::FileNotFound);
            return runResult;
        }
        const std::vector<double> frequencySamples = matDb.FrequencySamplesHz();
        std::ostringstream matLog;
        matLog << "材料数据库读取完成：" << config.material.material_database_file
               << "，材料 " << matDb.MaterialCount() << " 类";
        if (!frequencySamples.empty()) {
            matLog << "，表内频点 ";
            const std::size_t showCount = std::min<std::size_t>(frequencySamples.size(), 8U);
            for (std::size_t i = 0; i < showCount; ++i) {
                if (i > 0) matLog << "/";
                matLog << FormatFrequencyGHz(frequencySamples[i]);
            }
            if (frequencySamples.size() > showCount) matLog << "/...";
        }
        logger.Log(LogLevel::Info, "材料", matLog.str());

        if (matDb.IsFrequencyInTabulatedRange(config.em_solver.frequency_hz)) {
            logger.Log(LogLevel::Info, "材料",
                "当前仿真频点 " + FormatFrequencyGHz(config.em_solver.frequency_hz) +
                " 位于材料表范围内，材料查询使用对数-对数幂律插值。");
        } else {
            logger.Log(LogLevel::Warn, "材料",
                "当前仿真频点 " + FormatFrequencyGHz(config.em_solver.frequency_hz) +
                " 超出材料表范围；材料查询将使用 ITU-R P.2040 幂律模型进行表外外推。");
        }
    }

    if (!matDb.empty()) {
        const double fq = config.em_solver.frequency_hz;
        int matMissingCount = 0;
        int matSameCount = 0;
        for (Face& face : batch4Result.scene.faces) {
            if (!face.surface_material_name.empty()) {
                MaterialProps sp = matDb.QueryByName(face.surface_material_name, fq);
                face.surface_eps_r = sp.epsilon_r;
                face.surface_sigma = sp.sigma;
                if (sp.name.empty() && face.surface_material_name != "vacuum") {
                    ++matMissingCount;
                    if (matMissingCount <= 10) {
                        logger.Log(LogLevel::Warn, "材料",
                            "面元 #" + std::to_string(face.face_id) + " 的材料 '" +
                            face.surface_material_name + "' 不在数据库中，临时按真空处理。");
                    }
                }
            }
            if (face.transmission_enabled && face.dual_side_material_resolved &&
                !face.front_material_name.empty() && face.front_material_name == face.back_material_name) {
                ++matSameCount;
            }
        }
        if (matMissingCount > 0) {
            if (config.material.missing_material_policy == "strict") {
                logger.Log(LogLevel::Fatal, "材料",
                    std::to_string(matMissingCount) + " 个面元材料不在数据库中，当前策略为 strict。");
                runResult.exit_code = static_cast<int>(ErrorCode::ValidationFailed);
                return runResult;
            }
            logger.Log(LogLevel::Warn, "材料",
                std::to_string(matMissingCount) + " 个面元材料未在数据库中找到。");
        }
        if (matSameCount > 0) {
            logger.Log(LogLevel::Warn, "材料",
                std::to_string(matSameCount) + " 个透射面元的 front/back 材料相同。");
        }
    }

    if (config.pipeline.enable_stage0_precompute && batch4Result.scene.query) {
        SceneVisibilityBuilder::BuildAll(batch4Result.scene, *batch4Result.scene.query, config);
        std::ostringstream pvsLog;
        pvsLog << "可见性预计算完成：PVS " << batch4Result.scene.visibility.face_pvs.total_entries
               << " 条，EdgeAdj " << batch4Result.scene.visibility.edge_adjacency.total_edges
               << " 对，AngularGrid " << batch4Result.scene.visibility.angular_grid.CellCount()
               << " 个单元，耗时 " << batch4Result.scene.visibility.build_time_seconds << " s。";
        logger.Log(LogLevel::Info, "预处理", pvsLog.str());
    }

    const std::string reportDir = "output/" + config.app_runtime.run_id + "/reports";
    std::error_code ec;
    std::filesystem::create_directories(reportDir, ec);
    std::ofstream pf(reportDir + "/scene_preflight_report.json");
    if (pf.is_open()) {
        pf << "{\n"
           << "  \"scene\": \"" << JsonEscape(config.scene_import.source_file) << "\",\n"
           << "  \"faces\": " << batch4Result.scene.faces.size() << ",\n"
           << "  \"vertices\": " << batch4Result.scene.vertices.size() << ",\n"
           << "  \"wedges\": " << batch4Result.scene.wedges.size() << ",\n"
           << "  \"diffractable_wedges\": " << batch4Result.scene.acceleration.wedge_acceleration.diffractable_wedge_count << ",\n"
           << "  \"material_policy\": \"" << JsonEscape(config.material.missing_material_policy) << "\",\n"
           << "  \"material_db\": \"" << JsonEscape(config.material.material_database_file) << "\"\n"
           << "}\n";
    }

    const bool preciseEnabled = config.pipeline.enable_stage4_precise_em
        && config.em_solver.solver_mode != "Coverage";
    if (!preciseEnabled) {
        logger.Log(LogLevel::Fatal, "仿真",
            "v11 主链要求启用 P2P 精确仿真：请开启 precise EM，并使用非 Coverage 求解模式。");
        return runResult;
    }
    logger.Log(LogLevel::Info, "仿真", "P2P 点对点仿真已启用。");
    logger.Log(LogLevel::Info, "仿真", "当前仿真频点：" + FormatFrequencyGHz(config.em_solver.frequency_hz) + "。");
    LogSbrKeyParameters(logger, config);

    if (!config.v11_p2p_tasks.empty()) {
        const std::string baseRunId = config.app_runtime.run_id;
        std::set<std::string> txIds;
        std::set<std::string> rxIds;
        for (const P2PLinkTask& task : config.v11_p2p_tasks) {
            txIds.insert(task.tx.id);
            rxIds.insert(task.rx.id);
        }
        logger.Log(LogLevel::Info, "仿真",
            "天线任务：Tx " + std::to_string(txIds.size()) +
            " 个，Rx " + std::to_string(rxIds.size()) +
            " 个，共 " + std::to_string(config.v11_p2p_tasks.size()) + " 条链路。");
        logger.Log(LogLevel::Info, "仿真", "任务列表：" + BuildTaskListText(config.v11_p2p_tasks));

        bool anyTaskSucceeded = false;
        int succeededTasks = 0;
        for (const P2PLinkTask& task : config.v11_p2p_tasks) {
            logger.Log(LogLevel::Info, "任务", task.id + " 开始。");
            const RtClock::time_point taskStart = RtClock::now();
            AppConfig taskConfig = BuildConfigForP2PTask(config, task, baseRunId);
            const Point3 rxPoint = MakeVec3(task.rx.x, task.rx.y, task.rx.z);

            SearchEngineResult searchResult =
                RunSbrPointToPointSearch(taskConfig, batch4Result.scene, &matDb, rxPoint, logger, task.id);
            if (!searchResult.succeeded || searchResult.path_set.paths.empty()) {
                logger.Log(LogLevel::Error, "任务", task.id + " 寻径失败，跳过后续电磁计算。");
                continue;
            }

            const RtClock::time_point emStart = RtClock::now();
            A1RealChainRunResult a1Result =
                RunA1RealChain(taskConfig, batch4Result.scene, searchResult, logger, &matDb);
            logger.Log(LogLevel::Info, "电磁计算",
                task.id + " 电磁计算耗时 " + FormatSeconds(ElapsedSeconds(emStart, RtClock::now())) + "。");
            logger.Log(LogLevel::Info, "任务", task.id +
                (a1Result.succeeded ? " 完成" : " 电磁计算失败") +
                "，输出目录：output/" + taskConfig.app_runtime.run_id +
                "，任务耗时 " + FormatSeconds(ElapsedSeconds(taskStart, RtClock::now())) + "。");
            anyTaskSucceeded = anyTaskSucceeded || a1Result.succeeded;
            if (a1Result.succeeded) ++succeededTasks;
        }

        if (!anyTaskSucceeded) {
            logger.Log(LogLevel::Fatal, "仿真", "没有任何 P2P 链路完成。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "完成",
            "全部 P2P 仿真完成：成功 " + std::to_string(succeededTasks) +
            "/" + std::to_string(config.v11_p2p_tasks.size()) +
            "，总耗时 " + FormatSeconds(ElapsedSeconds(pipelineStart, RtClock::now())) + "。");
    } else if (config.path_search.rx_list.empty()) {
        const std::string taskLabel = "Tx-Rx";
        logger.Log(LogLevel::Info, "任务", taskLabel + " 开始。");
        const Point3 rxPoint = MakeVec3(config.path_search.rx_x,
                                        config.path_search.rx_y,
                                        config.path_search.rx_z);
        SearchEngineResult searchResult =
            RunSbrPointToPointSearch(config, batch4Result.scene, &matDb, rxPoint, logger, taskLabel);
        if (!searchResult.succeeded || searchResult.path_set.paths.empty()) {
            logger.Log(LogLevel::Fatal, "寻径", "SBR P2P 未产生几何路径。");
            return runResult;
        }

        const RtClock::time_point emStart = RtClock::now();
        A1RealChainRunResult a1Result =
            RunA1RealChain(config, batch4Result.scene, searchResult, logger, &matDb);
        if (!a1Result.succeeded) {
            logger.Log(LogLevel::Fatal, "电磁计算", "P2P 电磁计算失败。");
            return runResult;
        }
        logger.Log(LogLevel::Info, "电磁计算",
            taskLabel + " 电磁计算耗时 " + FormatSeconds(ElapsedSeconds(emStart, RtClock::now())) + "。");
    } else {
        logger.Log(LogLevel::Info, "仿真", "多 Rx P2P 批处理：" +
            std::to_string(config.path_search.rx_list.size()) + " 个 Rx。");
        const std::string origRunId = config.app_runtime.run_id;
        for (const auto& rxTarget : config.path_search.rx_list) {
            AppConfig taskConfig = config;
            taskConfig.app_runtime.run_id = origRunId + "/" + rxTarget.id;

            const Point3 rxPoint = MakeVec3(rxTarget.x, rxTarget.y, rxTarget.z);
            logger.Log(LogLevel::Info, "任务", rxTarget.id + " 开始。");
            SearchEngineResult searchResult =
                RunSbrPointToPointSearch(taskConfig, batch4Result.scene, &matDb, rxPoint, logger, rxTarget.id);
            if (!searchResult.succeeded || searchResult.path_set.paths.empty()) {
                logger.Log(LogLevel::Error, "任务", rxTarget.id + " 寻径失败，跳过。");
                continue;
            }

            A1RealChainRunResult a1Result =
                RunA1RealChain(taskConfig, batch4Result.scene, searchResult, logger, &matDb);
            logger.Log(LogLevel::Info, "任务", rxTarget.id +
                (a1Result.succeeded ? " 完成" : " 电磁计算失败") +
                "，输出目录：output/" + taskConfig.app_runtime.run_id);
        }
    }

    runResult.succeeded = true;
    runResult.exit_code = 0;
    runResult.completed_batch = 10;
    return runResult;
}

} // namespace rt
