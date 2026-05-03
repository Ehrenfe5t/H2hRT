// 文件目标：
// - 实现批次8多路径汇总与双模式求值验证输出函数。
//
// 主要功能：
// - 复用批次7路径级 EM 结果构建双模式汇总结果；
// - 输出 CIR / PDP / APS / Statistics / Coverage / ISAC 特征摘要；
// - 比较 PreciseEM 与 CoverageEM 差异，满足批次8检测要求。

#include "Batch8AggregateReporter.h"

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
            if (!ApplyReflectionInteraction(field, node))
            {
                return false;
            }
        }
        else if (node.interaction_type == InteractionType::Transmission)
        {
            if (!ApplyTransmissionInteraction(field, node))
            {
                return false;
            }
        }
        else if (node.interaction_type == InteractionType::Diffraction)
        {
            if (!ApplyDiffractionInteraction(field, node))
            {
                return false;
            }
        }
    }

    result = FinalizeAtReceiver(field, path);
    return result.valid;
}

GeometricPath BuildLosPath()
{
    GeometricPath path;
    path.path_id = 0;
    path.is_los = true;
    path.valid = true;

    PathNode tx;
    tx.interaction_type = InteractionType::Tx;
    tx.point.x = 1.0;
    tx.point.y = 1.0;
    tx.point.z = 1.0;
    tx.valid = true;

    PathNode rx;
    rx.interaction_type = InteractionType::Rx;
    rx.point.x = 3.0;
    rx.point.y = 1.0;
    rx.point.z = 1.0;
    rx.segment_length_from_previous = 2.0;
    rx.valid = true;

    path.nodes = { tx, rx };
    path.total_length = 2.0;
    return path;
}

bool BuildSingleReflectionPath(const Scene& scene, GeometricPath& path)
{
    for (const Face& face : scene.faces)
    {
        if (!face.reflection_enabled || face.degenerate)
        {
            continue;
        }
        path = GeometricPath{};
        path.path_id = 1;
        path.valid = true;

        PathNode tx;
        tx.interaction_type = InteractionType::Tx;
        tx.point = AddOffset(face.centroid, face.normal, 1.0);
        tx.valid = true;

        PathNode interaction;
        interaction.interaction_type = InteractionType::Reflection;
        interaction.object_id = face.object_id;
        interaction.face_id = face.face_id;
        interaction.point = face.centroid;
        interaction.surface_normal = face.normal;
        interaction.segment_length_from_previous = Length(Subtract(interaction.point, tx.point));
        interaction.valid = true;

        PathNode rx;
        rx.interaction_type = InteractionType::Rx;
        rx.point = AddOffset(face.centroid, face.normal, 1.0);
        rx.segment_length_from_previous = Length(Subtract(rx.point, interaction.point));
        rx.valid = true;

        path.nodes = { tx, interaction, rx };
        path.total_length = interaction.segment_length_from_previous + rx.segment_length_from_previous;
        return true;
    }
    return false;
}

bool BuildSingleTransmissionPath(const Scene& scene, GeometricPath& path)
{
    for (const Face& face : scene.faces)
    {
        if (!face.transmission_enabled || !face.dual_side_material_resolved || face.degenerate)
        {
            continue;
        }
        path = GeometricPath{};
        path.path_id = 2;
        path.valid = true;

        PathNode tx;
        tx.interaction_type = InteractionType::Tx;
        tx.point = AddOffset(face.centroid, face.normal, 1.0);
        tx.valid = true;

        PathNode interaction;
        interaction.interaction_type = InteractionType::Transmission;
        interaction.object_id = face.object_id;
        interaction.face_id = face.face_id;
        interaction.point = face.centroid;
        interaction.surface_normal = face.normal;
        interaction.segment_length_from_previous = Length(Subtract(interaction.point, tx.point));
        interaction.valid = true;

        PathNode rx;
        rx.interaction_type = InteractionType::Rx;
        rx.point = AddOffset(face.centroid, face.normal, -1.0);
        rx.segment_length_from_previous = Length(Subtract(rx.point, interaction.point));
        rx.valid = true;

        path.nodes = { tx, interaction, rx };
        path.total_length = interaction.segment_length_from_previous + rx.segment_length_from_previous;
        return true;
    }
    return false;
}

bool BuildSingleDiffractionPath(const Scene& scene, GeometricPath& path)
{
    if (scene.acceleration.wedge_acceleration.wedge_query_records.empty())
    {
        return false;
    }

    const WedgeQueryRecord& record = scene.acceleration.wedge_acceleration.wedge_query_records.front();
    path = GeometricPath{};
    path.path_id = 3;
    path.valid = true;

    PathNode tx;
    tx.interaction_type = InteractionType::Tx;
    tx.point = AddOffset(record.center_point, record.direction, -1.0);
    tx.valid = true;

    PathNode interaction;
    interaction.interaction_type = InteractionType::Diffraction;
    interaction.wedge_id = record.wedge_id;
    interaction.point = record.center_point;
    interaction.segment_length_from_previous = Length(Subtract(interaction.point, tx.point));
    interaction.valid = true;

    PathNode rx;
    rx.interaction_type = InteractionType::Rx;
    rx.point = AddOffset(record.center_point, record.direction, 1.0);
    rx.segment_length_from_previous = Length(Subtract(rx.point, interaction.point));
    rx.valid = true;

    path.nodes = { tx, interaction, rx };
    path.total_length = interaction.segment_length_from_previous + rx.segment_length_from_previous;
    return true;
}

EMPathResultSet BuildReferencePathResultSet(const AppConfig& config, const Scene& scene)
{
    EMPathResultSet set;
    EMPathResult result;

    const GeometricPath los = BuildLosPath();
    if (SolveSinglePathEM(config, scene, los, result))
    {
        set.results.push_back(result);
    }

    GeometricPath reflection;
    if (BuildSingleReflectionPath(scene, reflection) && SolveSinglePathEM(config, scene, reflection, result))
    {
        set.results.push_back(result);
    }

    GeometricPath transmission;
    if (BuildSingleTransmissionPath(scene, transmission) && SolveSinglePathEM(config, scene, transmission, result))
    {
        set.results.push_back(result);
    }

    GeometricPath diffraction;
    if (BuildSingleDiffractionPath(scene, diffraction) && SolveSinglePathEM(config, scene, diffraction, result))
    {
        set.results.push_back(result);
    }

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

bool LogAggregate(const std::string& name, const EMAggregateResult& result, Logger& logger)
{
    std::ostringstream stream;
    stream << name
           << ": paths=" << result.path_results.results.size()
           << ", cir_taps=" << result.cir.taps.size()
           << ", pdp_taps=" << result.pdp.taps.size()
           << ", aps_entries=" << result.aps.entries.size()
           << ", total_power=" << result.statistics.total_power_linear
           << ", coverage_power=" << result.coverage.total_received_power_linear
           << ", isac_paths=" << result.isac_features.path_count;
    logger.Log(LogLevel::Info, "Module5", stream.str());
    return !result.path_results.results.empty() && !result.pdp.taps.empty();
}

} // namespace

/// <summary>
/// 执行批次8多路径汇总与双模式自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次8自检通过；false 表示失败。</returns>
bool ReportBatch8AggregateSummary(const AppConfig& config, const Scene& scene, Logger& logger)
{
    const EMPathResultSet pathResults = BuildReferencePathResultSet(config, scene);
    const EMSolveProfile preciseProfile = BuildPreciseEMProfile();
    const EMSolveProfile coverageProfile = BuildCoverageEMProfile();

    const EMAggregateResult preciseResult = BuildAggregateResult(pathResults, preciseProfile);
    const EMAggregateResult coverageResult = BuildAggregateResult(pathResults, coverageProfile);

    const bool preciseOk = LogAggregate("PreciseEMAggregate", preciseResult, logger);
    const bool coverageOk = LogAggregate("CoverageEMAggregate", coverageResult, logger);

    std::ostringstream compareStream;
    compareStream << "EMProfileCompare: precise_total_power=" << preciseResult.statistics.total_power_linear
                  << ", coverage_total_power=" << coverageResult.statistics.total_power_linear
                  << ", coverage_paths=" << coverageResult.coverage.contributing_path_count;
    logger.Log(LogLevel::Info, "Module5", compareStream.str());

    return preciseOk && coverageOk;
}

} // namespace rt
