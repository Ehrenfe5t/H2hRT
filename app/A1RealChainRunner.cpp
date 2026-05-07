// A1 real production chain runner -- implementation.
// Takes the SearchEngine's real path set through per-path EM solving, builds both Precise and
// Coverage aggregate profiles, then exports results, validation, and regression reports.
// This is the primary production chain; it does NOT rely on hand-crafted reference paths.

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
/// Runs the full EM solve for a single geometric path (Tx antenna -> propagation -> Rx antenna).
/// </summary>
/// <param name="config">Application configuration.</param>
/// <param name="scene">Static scene with geometry and materials.</param>
/// <param name="path">Geometric ray path whose EM result is to be computed.</param>
/// <param name="result">[out] EM result for this path (field, delay, power, etc.).</param>
/// <param name="materialDb">Optional material database for dielectric constants.</param>
/// <returns>true if the EM solve completed and produced a valid result.</returns>
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

    // EM pipeline: prepare path, initialize TX field, walk interactions along the path.
    if (!PreparePathForEM(input))
    {
        return false;
    }

    FieldAccumulator field;
    if (!InitializeTxField(input, field))
    {
        return false;
    }

    // Walk each path segment: free-space attenuation followed by the interaction.
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
/// Builds the complete EM path result set by solving every path from the search engine.
/// Logs a warning for each path that fails the EM solve.
/// </summary>
EMPathResultSet BuildRealPathResultSet(const AppConfig& config, const Scene& scene, const SearchEngineResult& searchResult, Logger& logger, const MaterialDatabase* materialDb)
{
    EMPathResultSet set;
    set.from_search_engine = true;
    set.input_path_count = static_cast<int>(searchResult.path_set.paths.size());
    set.source_tag = searchResult.source_tag;

    for (const GeometricPath& path : searchResult.path_set.paths)
    {
        EMPathResult result;
        if (SolveSinglePathEM(config, scene, path, result, materialDb))
        {
            set.results.push_back(result);
        }
        else
        {
            std::ostringstream stream;
            stream << "A1RealChain: EM solve skipped/failed for path_id=" << path.path_id;
            logger.Log(LogLevel::Warn, "Module5", stream.str());
        }
    }

    set.valid_result_count = static_cast<int>(set.results.size());
    return set;
}

/// <summary>
/// Aggregates per-path EM results into CIR, PDP, APS, channel statistics, coverage,
/// and ISAC feature sets for a given solve profile.
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
    // Guard: SearchEngine must have produced a non-empty real path set.
    if (!searchResult.succeeded || searchResult.path_set.paths.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because SearchEngine produced no real path set.");
        return runResult;
    }

    // Step 1: Per-path EM solving (Module 5 real chain).
    runResult.path_result_set = BuildRealPathResultSet(config, scene, searchResult, logger, materialDb);
    if (runResult.path_result_set.results.empty())
    {
        logger.Log(LogLevel::Error, "A1", "A1 real chain aborted because real path set could not produce any valid EMPathResult.");
        return runResult;
    }

    // Step 2: Build aggregate results for both Precise and Coverage EM profiles.
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

    // Step 3: Module-6 real export: Paths, Channel, Coverage, ISAC, Visualization.
    bool exportSucceeded = true;
    exportSucceeded = ExportPaths(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportChannel(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportCoverage(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportISAC(context, runResult.export_bundle) && exportSucceeded;
    exportSucceeded = ExportVisualization(context, runResult.export_bundle) && exportSucceeded;

    // Step 4: Validation and regression close-loop.
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
