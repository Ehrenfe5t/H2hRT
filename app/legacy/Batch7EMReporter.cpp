// 文件目标：
// - 实现批次7路径级电磁主链验证输出函数。
//
// 主要功能：
// - 构造 LOS / 单反射 / 单透射 / 单绕射测试路径；
// - 调用模块5基础主链生成 EMPathResult；
// - 输出时延、相位、复振幅和功率摘要。

#include "Batch7EMReporter.h"

#include "../../core/antenna/AntennaFactory.h"
#include "../../core/em/ApplyDiffractionInteraction.h"
#include "../../core/em/ApplyFreeSpaceSegment.h"
#include "../../core/em/ApplyReflectionInteraction.h"
#include "../../core/em/ApplyTransmissionInteraction.h"
#include "../../core/em/EMSolverInput.h"
#include "../../core/em/FinalizeAtReceiver.h"
#include "../../core/em/InitializeTxField.h"
#include "../../core/em/PreparePathForEM.h"
#include "../../core/path/GeometricPath.h"

#include <cmath>
#include <sstream>

namespace rt {

namespace {

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
    const Point3 txPosition = !path.nodes.empty() ? path.nodes.front().point : Point3{};
    const Point3 rxPosition = !path.nodes.empty() ? path.nodes.back().point : Point3{};
    const AntennaModel tx = BuildTxAntennaModel(config, txPosition, "batch7-tx");
    const AntennaModel rx = BuildRxAntennaModel(config, rxPosition, "batch7-rx");
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

    path.nodes.push_back(tx);
    path.nodes.push_back(rx);
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

        PathNode refl;
        refl.interaction_type = InteractionType::Reflection;
        refl.object_id = face.object_id;
        refl.face_id = face.face_id;
        refl.point = face.centroid;
        refl.surface_normal = face.normal;
        refl.segment_length_from_previous = Length(Subtract(refl.point, tx.point));
        refl.valid = true;

        PathNode rx;
        rx.interaction_type = InteractionType::Rx;
        rx.point = AddOffset(face.centroid, face.normal, 1.0);
        rx.segment_length_from_previous = Length(Subtract(rx.point, refl.point));
        rx.valid = true;

        path.nodes = { tx, refl, rx };
        path.total_length = refl.segment_length_from_previous + rx.segment_length_from_previous;
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

        PathNode tran;
        tran.interaction_type = InteractionType::Transmission;
        tran.object_id = face.object_id;
        tran.face_id = face.face_id;
        tran.medium_in_id = face.front_medium_id;
        tran.medium_out_id = face.back_medium_id;
        tran.front_medium_id = face.front_medium_id;
        tran.back_medium_id = face.back_medium_id;
        tran.front_material_id = face.front_material_id;
        tran.back_material_id = face.back_material_id;
        tran.entered_from_front_side = true;
        tran.transmission_semantic_complete = face.transmission_semantic_complete;
        tran.point = face.centroid;
        tran.surface_normal = face.normal;
        tran.segment_length_from_previous = Length(Subtract(tran.point, tx.point));
        tran.valid = true;

        PathNode rx;
        rx.interaction_type = InteractionType::Rx;
        rx.point = AddOffset(face.centroid, face.normal, -1.0);
        rx.segment_length_from_previous = Length(Subtract(rx.point, tran.point));
        rx.valid = true;

        path.nodes = { tx, tran, rx };
        path.total_length = tran.segment_length_from_previous + rx.segment_length_from_previous;
        path.contains_transmission = true;
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

    PathNode diff;
    diff.interaction_type = InteractionType::Diffraction;
    diff.wedge_id = record.wedge_id;
    diff.point = record.center_point;
    diff.segment_length_from_previous = Length(Subtract(diff.point, tx.point));
    diff.valid = true;

    PathNode rx;
    rx.interaction_type = InteractionType::Rx;
    rx.point = AddOffset(record.center_point, record.direction, 1.0);
    rx.segment_length_from_previous = Length(Subtract(rx.point, diff.point));
    rx.valid = true;

    path.nodes = { tx, diff, rx };
    path.total_length = diff.segment_length_from_previous + rx.segment_length_from_previous;
    return true;
}

bool LogEmPathResult(const std::string& name, const EMPathResult& result, Logger& logger)
{
    std::ostringstream stream;
    stream << name << ": valid=" << (result.valid ? "true" : "false")
           << ", length=" << result.total_length_m
           << ", delay=" << result.delay_s
           << ", phase=" << result.phase_rad
           << ", power=" << result.power_linear
           << ", fs_loss_db=" << result.free_space_loss_db
           << ", pol_mag=" << result.polarization_magnitude
           << ", tx_antenna=" << result.tx_antenna_id
           << ", tx_source=" << result.tx_antenna_source_type
           << ", rx_antenna=" << result.rx_antenna_id
           << ", rx_source=" << result.rx_antenna_source_type
           << ", tx_semantic=" << (result.transmission_semantic_consumed ? "true" : "false")
           << ", medium_in=" << result.last_transmission_medium_in_id
           << ", medium_out=" << result.last_transmission_medium_out_id;
    logger.Log(result.valid ? LogLevel::Info : LogLevel::Error, "Module5", stream.str());
    return result.valid;
}

} // namespace

/// <summary>
/// 执行批次7路径级电磁主链自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次7自检通过；false 表示失败。</returns>
bool ReportBatch7EMSummary(const AppConfig& config, const Scene& scene, Logger& logger)
{
    bool allPassed = true;

    const GeometricPath losPath = BuildLosPath();
    EMPathResult losResult;
    allPassed = SolveSinglePathEM(config, scene, losPath, losResult) && LogEmPathResult("EMLosCheck", losResult, logger) && allPassed;

    GeometricPath reflectionPath;
    if (!config.path_search.enable_reflection)
    {
        logger.Log(LogLevel::Info, "Module5", "EMReflectionCheck: skipped because reflection is disabled by current config.");
    }
    else if (BuildSingleReflectionPath(scene, reflectionPath))
    {
        EMPathResult reflectionResult;
        allPassed = SolveSinglePathEM(config, scene, reflectionPath, reflectionResult) && LogEmPathResult("EMReflectionCheck", reflectionResult, logger) && allPassed;
    }
    else
    {
        logger.Log(LogLevel::Error, "Module5", "EMReflectionCheck: no reflection path could be constructed.");
        allPassed = false;
    }

    GeometricPath transmissionPath;
    if (!config.path_search.enable_transmission)
    {
        logger.Log(LogLevel::Info, "Module5", "EMTransmissionCheck: skipped because transmission is disabled by current config.");
    }
    else if (BuildSingleTransmissionPath(scene, transmissionPath))
    {
        EMPathResult transmissionResult;
        allPassed = SolveSinglePathEM(config, scene, transmissionPath, transmissionResult) && LogEmPathResult("EMTransmissionCheck", transmissionResult, logger) && allPassed;
    }
    else
    {
        logger.Log(LogLevel::Error, "Module5", "EMTransmissionCheck: no transmission path could be constructed.");
        allPassed = false;
    }

    GeometricPath diffractionPath;
    if (!config.path_search.enable_diffraction)
    {
        logger.Log(LogLevel::Info, "Module5", "EMDiffractionCheck: skipped because diffraction is disabled by current config.");
    }
    else if (BuildSingleDiffractionPath(scene, diffractionPath))
    {
        EMPathResult diffractionResult;
        allPassed = SolveSinglePathEM(config, scene, diffractionPath, diffractionResult) && LogEmPathResult("EMDiffractionCheck", diffractionResult, logger) && allPassed;
    }
    else
    {
        logger.Log(LogLevel::Error, "Module5", "EMDiffractionCheck: no diffraction path could be constructed.");
        allPassed = false;
    }

    return allPassed;
}

} // namespace rt
