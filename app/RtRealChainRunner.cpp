// RtRealChainRunner.cpp
// v11 EM handoff: consume SBR GeometricPath results, solve each path, aggregate CIR/PDP/APS/channel outputs.

#include "RtRealChainRunner.h"

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

} // namespace

// Public single-path EM solve entry. It consumes one GeometricPath and returns
// the corresponding EMPathResult.
bool SolveSinglePathEM(const AppConfig& config, const Scene& scene, const GeometricPath& path, EMPathResult& result, const MaterialDatabase* materialDb)
{
    return SolveSinglePathEM(config, scene, path, result, materialDb, nullptr);
}

// Variant used when the caller provides a per-Rx antenna override.
bool SolveSinglePathEM(const AppConfig& config, const Scene& scene, const GeometricPath& path,
                       EMPathResult& result, const MaterialDatabase* materialDb,
                       const RxTarget* rxTarget)
{
    EMSolverInput input;
    input.config = &config;
    input.scene = &scene;
    input.path = &path;
    input.material_db = materialDb;
    const Point3 txPosition = !path.nodes.empty() ? path.nodes.front().point : Point3{};
    const Point3 rxPosition = !path.nodes.empty() ? path.nodes.back().point : Point3{};
    const AntennaModel tx = BuildTxAntennaModel(config, txPosition, "a1-realchain-tx");
    AntennaModel rx;
    if (rxTarget) {
        rx = BuildRxAntennaModelWithOverride(config, *rxTarget, rxPosition, "a1-realchain-rx");
    } else {
        rx = BuildRxAntennaModel(config, rxPosition, "a1-realchain-rx");
    }
    input.tx_antenna = &tx;
    input.rx_antenna = &rx;

    if (!PreparePathForEM(input))
    {
        return false;
    }

    FieldAccumulator field;
    if (!InitializeTxField(input, field))
    {
        return false;
    }

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

namespace {

// Solve all geometric paths independently and collect valid EM path results.
EMPathResultSet BuildRealPathResultSet(const AppConfig& config, const Scene& scene, const SearchEngineResult& searchResult, Logger& logger, const MaterialDatabase* materialDb, const RxTarget* rxTarget = nullptr)
{
    EMPathResultSet set;
    set.from_search_engine = true;
    const int nPaths = static_cast<int>(searchResult.path_set.paths.size());
    set.input_path_count = nPaths;
    set.source_tag = searchResult.source_tag;

    const auto& paths = searchResult.path_set.paths;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (int i = 0; i < nPaths; ++i)
    {
        EMPathResult result;
        if (SolveSinglePathEM(config, scene, paths[i], result, materialDb, rxTarget))
        {
            result.source_tag = searchResult.source_tag;
#ifdef _OPENMP
#pragma omp critical
#endif
            set.results.push_back(result);
        }
    }

    set.valid_result_count = static_cast<int>(set.results.size());
    return set;
}

// Aggregate per-path EM results into CIR/PDP/APS/channel/coverage/ISAC outputs.
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
    if (!searchResult.succeeded || searchResult.path_set.paths.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because SearchEngine produced no real path set.");
        return runResult;
    }

    runResult.path_result_set = BuildRealPathResultSet(config, scene, searchResult, logger, materialDb, nullptr);
    if (runResult.path_result_set.results.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because real path set could not produce any valid EMPathResult.");
        return runResult;
    }

    runResult.precise_result = BuildAggregateResult(runResult.path_result_set, BuildPreciseEMProfile());
    runResult.coverage_result = BuildAggregateResult(runResult.path_result_set, BuildCoverageEMProfile());

    if (config.frequency_sweep.enabled) {
        if (config.frequency_sweep.mode == "frequency_sweep_em") {
            runResult.broadband_result = BuildBroadbandCFR_FrequencySweep(
                runResult.path_result_set,
                config.frequency_sweep,
                config.channel_observation,
                materialDb,
                &scene);
        } else {
            runResult.broadband_result = BuildBroadbandCFR_FixedGain(
                runResult.path_result_set,
                config.frequency_sweep,
                config.channel_observation);
        }
        if (runResult.broadband_result.valid) {
            logger.Log(LogLevel::Info, "A1",
                "宽带 CFR 构建完成：" + std::to_string(runResult.broadband_result.cfr.size()) +
                " freq points, " + std::to_string(runResult.broadband_result.observed_cir.taps.size()) +
                " CIR taps");
        }
    }

    ResultExportContext context;
    context.config = &config;
    context.search_result = &searchResult;
    context.precise_result = &runResult.precise_result;
    context.coverage_result = &runResult.coverage_result;
    context.broadband_result = &runResult.broadband_result; // v9 Stage5
    context.real_chain_enabled = true;
    context.primary_input_source = searchResult.source_tag;
    context.export_root_directory = "output/" + config.app_runtime.run_id;

    runResult.export_bundle.root_directory = context.export_root_directory;
    runResult.export_bundle.primary_input_source = context.primary_input_source;
    runResult.export_bundle.search_path_count = static_cast<int>(searchResult.path_set.paths.size());
    runResult.export_bundle.em_path_result_count = static_cast<int>(runResult.path_result_set.results.size());
    runResult.export_bundle.used_reference_path_fallback = false;

    bool exportSucceeded = true;
    exportSucceeded = ExportPaths(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportChannel(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportCoverage(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportISAC(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportVisualization(context, runResult.export_bundle) && exportSucceeded;

    runResult.validation_report = BuildValidationReport(runResult.export_bundle, context);
    exportSucceeded = ExportValidationReport(runResult.validation_report, context, runResult.export_bundle) && exportSucceeded;
    runResult.regression_report = BuildRegressionReport(context);
    exportSucceeded = ExportRegressionReport(runResult.regression_report, context, runResult.export_bundle) && exportSucceeded;

    runResult.export_bundle.succeeded = exportSucceeded;
    runResult.succeeded = !runResult.path_result_set.results.empty();

    std::ostringstream stream;
    stream << "电磁计算与结果导出完成：输入路径 " << runResult.export_bundle.search_path_count
           << "，有效电磁结果 " << runResult.export_bundle.em_path_result_count
           << "，导出文件 " << runResult.export_bundle.exported_files.size()
           << "，验证" << (runResult.validation_report.passed ? "通过" : "未通过")
           << "，回归阻塞=" << (runResult.regression_report.has_blocking_diff ? "是" : "否");
    logger.Log(runResult.succeeded ? LogLevel::Info : LogLevel::Error, "电磁计算", stream.str());

    return runResult;
}

} // namespace rt
