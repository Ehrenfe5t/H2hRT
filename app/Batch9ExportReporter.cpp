// Batch-9 export, validation, and regression reporter -- implementation.
// NOTE: Transitional / legacy module. Builds hand-crafted reference paths (LOS, reflection,
// transmission, diffraction) and runs them through the full EM + export pipeline for
// cross-validating the A1 real chain. This should be phased out once A1 is stable.

#include "Batch9ExportReporter.h"

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
#include "../core/em/EMProfile.h"
#include "../core/em/EMSolverInput.h"
#include "../core/em/FinalizeAtReceiver.h"
#include "../core/em/InitializeTxField.h"
#include "../core/em/PreparePathForEM.h"
#include "../core/em/PreciseEMProfile.h"
#include "../core/result/ExportBundle.h"
#include "../core/result/ExportChannel.h"
#include "../core/result/ExportCoverage.h"
#include "../core/result/ExportISAC.h"
#include "../core/result/ExportPaths.h"
#include "../core/result/ExportVisualization.h"
#include "../core/result/RegressionReportWriter.h"
#include "../core/result/ResultExportContext.h"
#include "../core/result/ValidationReportWriter.h"

#include <cmath>
#include <sstream>

namespace rt {

namespace {

/// <summary>Offsets a point along a direction by a given scale factor.</summary>
Point3 AddOffset(const Point3& point, const Vec3& direction, double scale)
{
    Point3 value;
    value.x = point.x + direction.x * scale;
    value.y = point.y + direction.y * scale;
    value.z = point.z + direction.z * scale;
    return value;
}

/// <summary>
/// Runs the full EM solve for a single reference path (legacy: uses batch9-tx/batch9-rx antenna tags).
/// </summary>
bool SolveSinglePathEM(const AppConfig& config, const Scene& scene, const GeometricPath& path, EMPathResult& result)
{
    EMSolverInput input;
    input.config = &config;
    input.scene = &scene;
    input.path = &path;
    const Point3 txPosition = !path.nodes.empty() ? path.nodes.front().point : Point3{};
    const Point3 rxPosition = !path.nodes.empty() ? path.nodes.back().point : Point3{};
    const AntennaModel tx = BuildTxAntennaModel(config, txPosition, "batch9-tx");
    const AntennaModel rx = BuildRxAntennaModel(config, rxPosition, "batch9-rx");
    input.tx_antenna = &tx;
    input.rx_antenna = &rx;
    if (!PreparePathForEM(input)) return false;
    FieldAccumulator field;
    if (!InitializeTxField(input, field)) return false;
    for (std::size_t i = 1; i < path.nodes.size(); ++i)
    {
        const PathNode& node = path.nodes[i];
        if (!ApplyFreeSpaceSegment(field, node.segment_length_from_previous)) return false;
        if (node.interaction_type == InteractionType::Reflection)
        {
            if (!ApplyReflectionInteraction(field, node)) return false;
        }
        else if (node.interaction_type == InteractionType::Transmission)
        {
            if (!ApplyTransmissionInteraction(field, node)) return false;
        }
        else if (node.interaction_type == InteractionType::Diffraction)
        {
            if (!ApplyDiffractionInteraction(field, node)) return false;
        }
    }
    result = FinalizeAtReceiver(field, path);
    return result.valid;
}

/// <summary>Builds a hard-coded LOS reference path between (1,1,1) and (3,1,1).</summary>
GeometricPath BuildLosPath()
{
    GeometricPath path;
    path.path_id = 0; path.is_los = true; path.valid = true;
    PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point.x = 1.0; tx.point.y = 1.0; tx.point.z = 1.0; tx.valid = true;
    PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point.x = 3.0; rx.point.y = 1.0; rx.point.z = 1.0; rx.segment_length_from_previous = 2.0; rx.valid = true;
    path.nodes = { tx, rx }; path.total_length = 2.0;
    return path;
}

/// <summary>Builds a single-reflection reference path using the first reflection-enabled face.</summary>
bool BuildSingleReflectionPath(const Scene& scene, GeometricPath& path)
{
    for (const Face& face : scene.faces)
    {
        if (!face.reflection_enabled || face.degenerate) continue;
        path = GeometricPath{}; path.path_id = 1; path.valid = true;
        PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point = AddOffset(face.centroid, face.normal, 1.0); tx.valid = true;
        PathNode n; n.interaction_type = InteractionType::Reflection; n.object_id = face.object_id; n.face_id = face.face_id; n.point = face.centroid; n.surface_normal = face.normal; n.segment_length_from_previous = Length(Subtract(n.point, tx.point)); n.valid = true;
        PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point = AddOffset(face.centroid, face.normal, 1.0); rx.segment_length_from_previous = Length(Subtract(rx.point, n.point)); rx.valid = true;
        path.nodes = { tx, n, rx }; path.total_length = n.segment_length_from_previous + rx.segment_length_from_previous; return true;
    }
    return false;
}

/// <summary>Builds a single-transmission reference path using the first transmission-enabled face.</summary>
bool BuildSingleTransmissionPath(const Scene& scene, GeometricPath& path)
{
    for (const Face& face : scene.faces)
    {
        if (!face.transmission_enabled || !face.dual_side_material_resolved || face.degenerate) continue;
        path = GeometricPath{}; path.path_id = 2; path.valid = true;
        PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point = AddOffset(face.centroid, face.normal, 1.0); tx.valid = true;
        PathNode n; n.interaction_type = InteractionType::Transmission; n.object_id = face.object_id; n.face_id = face.face_id; n.point = face.centroid; n.surface_normal = face.normal; n.segment_length_from_previous = Length(Subtract(n.point, tx.point)); n.valid = true;
        n.medium_in_id = face.front_medium_id; n.medium_out_id = face.back_medium_id; n.front_medium_id = face.front_medium_id; n.back_medium_id = face.back_medium_id; n.front_material_id = face.front_material_id; n.back_material_id = face.back_material_id; n.entered_from_front_side = true; n.transmission_semantic_complete = face.transmission_semantic_complete;
        PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point = AddOffset(face.centroid, face.normal, -1.0); rx.segment_length_from_previous = Length(Subtract(rx.point, n.point)); rx.valid = true;
        path.nodes = { tx, n, rx }; path.total_length = n.segment_length_from_previous + rx.segment_length_from_previous; path.contains_transmission = true; return true;
    }
    return false;
}

/// <summary>Builds a single-diffraction reference path from the first wedge query record.</summary>
bool BuildSingleDiffractionPath(const Scene& scene, GeometricPath& path)
{
    if (scene.acceleration.wedge_acceleration.wedge_query_records.empty()) return false;
    const WedgeQueryRecord& record = scene.acceleration.wedge_acceleration.wedge_query_records.front();
    path = GeometricPath{}; path.path_id = 3; path.valid = true;
    PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point = AddOffset(record.center_point, record.direction, -1.0); tx.valid = true;
    PathNode n; n.interaction_type = InteractionType::Diffraction; n.wedge_id = record.wedge_id; n.point = record.center_point; n.segment_length_from_previous = Length(Subtract(n.point, tx.point)); n.valid = true;
    PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point = AddOffset(record.center_point, record.direction, 1.0); rx.segment_length_from_previous = Length(Subtract(rx.point, n.point)); rx.valid = true;
    path.nodes = { tx, n, rx }; path.total_length = n.segment_length_from_previous + rx.segment_length_from_previous; return true;
}

/// <summary>
/// Assembles hand-crafted reference paths (LOS, reflection, transmission, diffraction)
/// and solves each one through EM, yielding the legacy reference result set.
/// </summary>
EMPathResultSet BuildReferencePathResultSet(const AppConfig& config, const Scene& scene)
{
    EMPathResultSet set;
    EMPathResult result;
    GeometricPath path = BuildLosPath();
    if (SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (config.path_search.enable_reflection && BuildSingleReflectionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (config.path_search.enable_transmission && BuildSingleTransmissionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (config.path_search.enable_diffraction && BuildSingleDiffractionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
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

/// <summary>
/// Legacy Batch-9 export, validation, and regression self-check.
/// Builds hand-crafted reference paths, runs the EM + export pipeline, and logs
/// a detailed summary including exported file count, validation status, and regression diffs.
/// </summary>
/// <param name="config">Application configuration.</param>
/// <param name="scene">Static scene with closed-loop geometry.</param>
/// <param name="logger">Unified logger.</param>
/// <returns>true if Batch-9 self-check passed; false otherwise.</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger)
{
    // Build hand-crafted reference paths and solve them through EM.
    const EMPathResultSet pathResults = BuildReferencePathResultSet(config, scene);
    const EMAggregateResult preciseResult = BuildAggregateResult(pathResults, BuildPreciseEMProfile());
    const EMAggregateResult coverageResult = BuildAggregateResult(pathResults, BuildCoverageEMProfile());

    ResultExportContext context;
    context.config = &config;
    context.precise_result = &preciseResult;
    context.coverage_result = &coverageResult;
    context.export_root_directory = config.output.output_directory + "/batch9_exports";
    context.export_purpose = "module6_delivery_export";
    context.handoff_view_name = "a8_handoff_view";

    ExportBundle bundle;
    bundle.root_directory = context.export_root_directory;
    bundle.primary_input_source = "antenna_model_formal_input";
    bundle.export_purpose = context.export_purpose;
    bundle.export_view_name = context.handoff_view_name;

    // Export all result types: Paths, Channel, Coverage, ISAC, Visualization.
    bool succeeded = true;
    succeeded = ExportPaths(context, bundle) && succeeded;
    succeeded = ExportChannel(context, bundle) && succeeded;
    succeeded = ExportCoverage(context, bundle) && succeeded;
    succeeded = ExportISAC(context, bundle) && succeeded;
    succeeded = ExportVisualization(context, bundle) && succeeded;

    // Validation and regression close-loop.
    const ValidationReport validationReport = BuildValidationReport(bundle, context);
    succeeded = ExportValidationReport(validationReport, context, bundle) && succeeded;
    const RegressionReport regressionReport = BuildRegressionReport(context);
    succeeded = ExportRegressionReport(regressionReport, context, bundle) && succeeded;

    bundle.succeeded = succeeded && validationReport.passed && !regressionReport.has_blocking_diff;

    std::ostringstream summary;
    summary << "Batch9Export: succeeded=" << (bundle.succeeded ? "true" : "false")
            << ", exported_files=" << bundle.exported_files.size()
            << ", validation_passed=" << (validationReport.passed ? "true" : "false")
            << ", validation_failed_items=" << validationReport.failed_item_count
            << ", validation_failed_module=" << validationReport.failed_module
            << ", regression_diff_items=" << regressionReport.diff_item_count
            << ", regression_warnings=" << regressionReport.warning_count
            << ", regression_blockings=" << regressionReport.blocking_count
            << ", regression_blocking_module=" << regressionReport.blocking_module
            << ", regression_blocking=" << (regressionReport.has_blocking_diff ? "true" : "false")
            << ", baseline_name=" << regressionReport.baseline_name
            << ", primary_input_source=" << bundle.primary_input_source;
    logger.Log(bundle.succeeded ? LogLevel::Info : LogLevel::Error, "Module6", summary.str());

    return bundle.succeeded;
}

} // namespace rt
