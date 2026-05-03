// 文件目标：
// - 实现批次9结果表达、验证与回归闭环验证输出函数。
//
// 主要功能：
// - 复用批次8聚合结果构建 ExportBundle；
// - 导出路径、信道、覆盖、通感、可视化和报告文件；
// - 输出验证报告与回归报告摘要。

#include "Batch9ExportReporter.h"

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

Vec3 Subtract(const Point3& a, const Point3& b)
{
    Vec3 value;
    value.x = a.x - b.x;
    value.y = a.y - b.y;
    value.z = a.z - b.z;
    return value;
}

double Length(const Vec3& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

Point3 AddOffset(const Point3& point, const Vec3& direction, double scale)
{
    Point3 value;
    value.x = point.x + direction.x * scale;
    value.y = point.y + direction.y * scale;
    value.z = point.z + direction.z * scale;
    return value;
}

bool SolveSinglePathEM(const AppConfig& config, const Scene& scene, const GeometricPath& path, EMPathResult& result)
{
    EMSolverInput input;
    input.config = &config;
    input.scene = &scene;
    input.path = &path;
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

GeometricPath BuildLosPath()
{
    GeometricPath path;
    path.path_id = 0; path.is_los = true; path.valid = true;
    PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point.x = 1.0; tx.point.y = 1.0; tx.point.z = 1.0; tx.valid = true;
    PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point.x = 3.0; rx.point.y = 1.0; rx.point.z = 1.0; rx.segment_length_from_previous = 2.0; rx.valid = true;
    path.nodes = { tx, rx }; path.total_length = 2.0;
    return path;
}

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

bool BuildSingleTransmissionPath(const Scene& scene, GeometricPath& path)
{
    for (const Face& face : scene.faces)
    {
        if (!face.transmission_enabled || !face.dual_side_material_resolved || face.degenerate) continue;
        path = GeometricPath{}; path.path_id = 2; path.valid = true;
        PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point = AddOffset(face.centroid, face.normal, 1.0); tx.valid = true;
        PathNode n; n.interaction_type = InteractionType::Transmission; n.object_id = face.object_id; n.face_id = face.face_id; n.point = face.centroid; n.surface_normal = face.normal; n.segment_length_from_previous = Length(Subtract(n.point, tx.point)); n.valid = true;
        PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point = AddOffset(face.centroid, face.normal, -1.0); rx.segment_length_from_previous = Length(Subtract(rx.point, n.point)); rx.valid = true;
        path.nodes = { tx, n, rx }; path.total_length = n.segment_length_from_previous + rx.segment_length_from_previous; return true;
    }
    return false;
}

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

EMPathResultSet BuildReferencePathResultSet(const AppConfig& config, const Scene& scene)
{
    EMPathResultSet set;
    EMPathResult result;
    GeometricPath path = BuildLosPath();
    if (SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (BuildSingleReflectionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (BuildSingleTransmissionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    if (BuildSingleDiffractionPath(scene, path) && SolveSinglePathEM(config, scene, path, result)) set.results.push_back(result);
    return set;
}

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
/// 执行批次9结果表达、验证与回归自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次9自检通过；false 表示失败。</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger)
{
    const EMPathResultSet pathResults = BuildReferencePathResultSet(config, scene);
    const EMAggregateResult preciseResult = BuildAggregateResult(pathResults, BuildPreciseEMProfile());
    const EMAggregateResult coverageResult = BuildAggregateResult(pathResults, BuildCoverageEMProfile());

    ResultExportContext context;
    context.config = &config;
    context.precise_result = &preciseResult;
    context.coverage_result = &coverageResult;
    context.export_root_directory = config.output.output_directory + "/batch9_exports";

    ExportBundle bundle;
    bundle.root_directory = context.export_root_directory;

    bool succeeded = true;
    succeeded = ExportPaths(context, bundle) && succeeded;
    succeeded = ExportChannel(context, bundle) && succeeded;
    succeeded = ExportCoverage(context, bundle) && succeeded;
    succeeded = ExportISAC(context, bundle) && succeeded;
    succeeded = ExportVisualization(context, bundle) && succeeded;

    const ValidationReport validationReport = BuildValidationReport(bundle, context);
    succeeded = ExportValidationReport(validationReport, context, bundle) && succeeded;
    const RegressionReport regressionReport = BuildRegressionReport(context);
    succeeded = ExportRegressionReport(regressionReport, context, bundle) && succeeded;

    bundle.succeeded = succeeded && validationReport.passed && !regressionReport.has_blocking_diff;

    std::ostringstream summary;
    summary << "Batch9Export: succeeded=" << (bundle.succeeded ? "true" : "false")
            << ", exported_files=" << bundle.exported_files.size()
            << ", validation_passed=" << (validationReport.passed ? "true" : "false")
            << ", regression_blocking=" << (regressionReport.has_blocking_diff ? "true" : "false");
    logger.Log(bundle.succeeded ? LogLevel::Info : LogLevel::Error, "Module6", summary.str());

    return bundle.succeeded;
}

} // namespace rt
