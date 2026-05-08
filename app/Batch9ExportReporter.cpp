// ───────────────────────────────────────────────────────────────────
// 文件: Batch9ExportReporter.cpp
// 用途: Batch-9导出/验证/回归报告器实现。
//       注意: 过渡/遗留模块。构建手工参考路径(LOS/反射/透射/绕射)并通过完整
//       EM+导出管线运行，用于与A1真实链交叉验证。A1稳定后可逐步淘汰。
// 所属模块: 应用层 (遗留资产)
// ───────────────────────────────────────────────────────────────────

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

/// <summary>沿方向向量按给定比例偏移点坐标。</summary>
Point3 AddOffset(const Point3& point, const Vec3& direction, double scale)
{
    Point3 value;
    value.x = point.x + direction.x * scale;
    value.y = point.y + direction.y * scale;
    value.z = point.z + direction.z * scale;
    return value;
}

/// <summary>
/// 对单条参考路径执行完整EM求解 (遗留: 使用batch9-tx/batch9-rx天线标签)。
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

/// <summary>构建(1,1,1)到(3,1,1)之间的硬编码LOS参考路径。</summary>
GeometricPath BuildLosPath()
{
    GeometricPath path;
    path.path_id = 0; path.is_los = true; path.valid = true;
    PathNode tx; tx.interaction_type = InteractionType::Tx; tx.point.x = 1.0; tx.point.y = 1.0; tx.point.z = 1.0; tx.valid = true;
    PathNode rx; rx.interaction_type = InteractionType::Rx; rx.point.x = 3.0; rx.point.y = 1.0; rx.point.z = 1.0; rx.segment_length_from_previous = 2.0; rx.valid = true;
    path.nodes = { tx, rx }; path.total_length = 2.0;
    return path;
}

/// <summary>使用第一个反射使能面元构建单次反射参考路径。</summary>
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

/// <summary>使用第一个透射使能面元构建单次透射参考路径。</summary>
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

/// <summary>从第一个楔边查询记录构建单次绕射参考路径。</summary>
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
/// 组装手工参考路径 (LOS/反射/透射/绕射) 并对每条执行EM求解, 产出遗留参考结果集。
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

/// <summary>
/// 遗留Batch-9导出/验证/回归自检。
/// 构建手工参考路径, 运行EM+导出管线, 输出含导出文件数/验证状态/回归差异的详细摘要。
/// </summary>
/// <returns>Batch-9自检通过返回true; 否则返回false。</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger)
{
    // 构建手工参考路径并通过EM求解
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

    // 导出全部结果类型: 路径/信道/覆盖/ISAC/可视化
    bool succeeded = true;
    succeeded = ExportPaths(context, bundle) && succeeded;
    succeeded = ExportChannel(context, bundle) && succeeded;
    succeeded = ExportCoverage(context, bundle) && succeeded;
    succeeded = ExportISAC(context, bundle) && succeeded;
    succeeded = ExportVisualization(context, bundle) && succeeded;

    // 验证与回归闭环
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
