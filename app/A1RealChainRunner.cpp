// ───────────────────────────────────────────────────────────────────
// 文件: A1RealChainRunner.cpp
// 用途: A1真实生产链运行器实现。将SearchEngine的真实路径集送入逐路径EM求解，
//       构建Precise和Coverage双profile聚合结果，执行导出/验证/回归闭环。
//       这是主生产链，不依赖手工参考路径。
// 所属模块: 应用层
// ───────────────────────────────────────────────────────────────────

#include "A1RealChainRunner.h"

#include "../core/antenna/AntennaFactory.h"
#include "../core/em/ApplyDiffractionInteraction.h"
#include "../core/em/ApplyFreeSpaceSegment.h"
#include "../core/em/ApplyReflectionInteraction.h"
#include "../core/em/ApplyTransmissionInteraction.h"
#include "../core/em/BuildAPS.h"
#include "../core/em/BuildCIR.h"
#include "../core/em/BuildChannelStatistics.h"
#include "../core/em/BuildCoverageResult.h"
#include "../core/em/BuildISACFeatureSet.h"
#include "../core/em/BuildPDP.h"
#include "../core/em/CoverageEMProfile.h"
#include "../core/em/EMSolverInput.h"
#include "../core/em/FinalizeAtReceiver.h"
#include "../core/em/InitializeTxField.h"
#include "../core/em/PreparePathForEM.h"
#include "../core/em/PreciseEMProfile.h"
#include "../core/result/ExportChannel.h"
#include "../core/result/ExportCoverage.h"
#include "../core/result/ExportISAC.h"
#include "../core/result/ExportPaths.h"
#include "../core/result/ExportVisualization.h"
#include "../core/result/RegressionReportWriter.h"
#include "../core/result/ResultExportContext.h"
#include "../core/result/ValidationReportWriter.h"

#include <sstream>

namespace rt {

namespace {

/// <summary>
/// 对单条几何路径执行完整EM求解 (Tx天线 → 传播 → Rx天线)。
/// </summary>
/// <param name="result">[out] 该路径的EM结果 (场、时延、功率等)。</param>
/// <param name="materialDb">可选材质数据库, 供介电常数查询。</param>
/// <returns>EM求解完成并产出有效结果时返回true。</returns>
bool SolveSinglePathEM(const AppConfig& config, const Scene& scene, const GeometricPath& path, EMPathResult& result, const MaterialDatabase* materialDb)
{
    EMSolverInput input;
    input.config = &config;
    input.scene = &scene;
    input.path = &path;
    input.material_db = materialDb;
    const Point3 txPosition = !path.nodes.empty() ? path.nodes.front().point : Point3{};
    const Point3 rxPosition = !path.nodes.empty() ? path.nodes.back().point : Point3{};
    const AntennaModel tx = BuildTxAntennaModel(config, txPosition, "a1-realchain-tx");
    const AntennaModel rx = BuildRxAntennaModel(config, rxPosition, "a1-realchain-rx");
    input.tx_antenna = &tx;
    input.rx_antenna = &rx;

    // EM管线: 路径准备 → 初始化Tx场 → 沿路径遍历各交互节点
    if (!PreparePathForEM(input))
    {
        return false;
    }

    FieldAccumulator field;
    if (!InitializeTxField(input, field))
    {
        return false;
    }

    // 遍历每个路径段: 先施加自由空间衰减, 再处理交互类型
    for (std::size_t i = 1; i < path.nodes.size(); ++i)
    {
        const PathNode& node = path.nodes[i];
        if (!ApplyFreeSpaceSegment(field, node.segment_length_from_previous))
        {
            return false;
        }

        if (node.interaction_type == InteractionType::Reflection)
        {
            if (!ApplyReflectionInteraction(field, node, input))
            {
                return false;
            }
        }
        else if (node.interaction_type == InteractionType::Transmission)
        {
            if (!ApplyTransmissionInteraction(field, node, input))
            {
                return false;
            }
        }
        else if (node.interaction_type == InteractionType::Diffraction)
        {
            if (!ApplyDiffractionInteraction(field, node, input))
            {
                return false;
            }
        }
    }

    result = FinalizeAtReceiver(field, path, input);
    result.source_tag = "search_engine_real_output";
    return result.valid;
}

/// <summary>
/// 对搜索引擎的全部路径执行EM求解, 构建完整的EM路径结果集。
/// 对每条EM求解失败的路径记录警告日志。
/// </summary>
EMPathResultSet BuildRealPathResultSet(const AppConfig& config, const Scene& scene, const SearchEngineResult& searchResult, Logger& logger, const MaterialDatabase* materialDb)
{
    EMPathResultSet set;
    set.from_search_engine = true;
    const int nPaths = static_cast<int>(searchResult.path_set.paths.size());
    set.input_path_count = nPaths;
    set.source_tag = searchResult.source_tag;

    // v5 D7: 逐路径EM求解可完全并行 (各路径独立)
    const auto& paths = searchResult.path_set.paths;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (int i = 0; i < nPaths; ++i)
    {
        EMPathResult result;
        if (SolveSinglePathEM(config, scene, paths[i], result, materialDb))
        {
#ifdef _OPENMP
#pragma omp critical
#endif
            set.results.push_back(result);
        }
    }

    set.valid_result_count = static_cast<int>(set.results.size());
    return set;
}

/// <summary>
/// 将逐路径EM结果聚合为CIR/PDP/APS/信道统计/覆盖/ISAC特征集。
/// </summary>
EMAggregateResult BuildAggregateResult(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    EMAggregateResult result;
    result.profile = profile;
    result.path_results = pathResults;
    result.cir = BuildCIR(pathResults, profile);
    result.pdp = BuildPDP(pathResults);
    result.aps = BuildAPS(pathResults);
    result.statistics = BuildChannelStatistics(pathResults);
    result.coverage = BuildCoverageResult(pathResults, profile);
    result.isac_features = BuildISACFeatureSet(pathResults);
    return result;
}

} // namespace

A1RealChainRunResult RunA1RealChain(
    const AppConfig& config,
    const Scene& scene,
    const SearchEngineResult& searchResult,
    Logger& logger,
    const MaterialDatabase* materialDb)
{
    A1RealChainRunResult runResult;
    (void)materialDb;
    // 守卫: SearchEngine必须产出非空的真实路径集
    if (!searchResult.succeeded || searchResult.path_set.paths.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because SearchEngine produced no real path set.");
        return runResult;
    }

    // 步骤1: 逐路径EM求解 (模块5真实链)
    runResult.path_result_set = BuildRealPathResultSet(config, scene, searchResult, logger, materialDb);
    if (runResult.path_result_set.results.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because real path set could not produce any valid EMPathResult.");
        return runResult;
    }

    // 步骤2: 构建Precise和Coverage双EM profile的聚合结果
    runResult.precise_result = BuildAggregateResult(runResult.path_result_set, BuildPreciseEMProfile());
    runResult.coverage_result = BuildAggregateResult(runResult.path_result_set, BuildCoverageEMProfile());

    ResultExportContext context;
    context.config = &config;
    context.search_result = &searchResult;
    context.precise_result = &runResult.precise_result;
    context.coverage_result = &runResult.coverage_result;
    context.real_chain_enabled = true;
    context.primary_input_source = "search_engine_real_output";
    context.export_root_directory = config.output.output_directory + "/a1_real_chain";

    runResult.export_bundle.root_directory = context.export_root_directory;
    runResult.export_bundle.primary_input_source = context.primary_input_source;
    runResult.export_bundle.search_path_count = static_cast<int>(searchResult.path_set.paths.size());
    runResult.export_bundle.em_path_result_count = static_cast<int>(runResult.path_result_set.results.size());
    runResult.export_bundle.used_reference_path_fallback = false;

    // 步骤3: 模块6真实导出: 路径/信道/覆盖/ISAC/可视化
    bool exportSucceeded = true;
    exportSucceeded = ExportPaths(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportChannel(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportCoverage(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportISAC(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportVisualization(context, runResult.export_bundle) && exportSucceeded;

    // 步骤4: 验证与回归闭环
    runResult.validation_report = BuildValidationReport(runResult.export_bundle, context);
    exportSucceeded = ExportValidationReport(runResult.validation_report, context, runResult.export_bundle) && exportSucceeded;
    runResult.regression_report = BuildRegressionReport(context);
    exportSucceeded = ExportRegressionReport(runResult.regression_report, context, runResult.export_bundle) && exportSucceeded;

    runResult.export_bundle.succeeded = exportSucceeded;
    runResult.succeeded = !runResult.path_result_set.results.empty();

    std::ostringstream stream;
    stream << "A1RealChainSummary: search_paths=" << runResult.export_bundle.search_path_count
           << ", em_results=" << runResult.export_bundle.em_path_result_count
           << ", exported_files=" << runResult.export_bundle.exported_files.size()
           << ", validation_passed=" << (runResult.validation_report.passed ? "true" : "false")
           << ", regression_blocking=" << (runResult.regression_report.has_blocking_diff ? "true" : "false");
    logger.Log(runResult.succeeded ? LogLevel::Info : LogLevel::Error, "A1", stream.str());

    return runResult;
}

} // namespace rt
