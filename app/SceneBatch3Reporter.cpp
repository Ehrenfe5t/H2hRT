// 文件目标：
// - 实现批次3场景拓扑、诊断与加速结构验证输出函数。
//
// 主要功能：
// - 输出拓扑统计、诊断摘要与 BVH 构建摘要；
// - 满足批次3对非流形边、未解析面与 BVH 节点统计的检查要求；
// - 为后续 SceneQuery 与模块4接入提供前置可观测性。

#include "SceneBatch3Reporter.h"

#include <sstream>

namespace rt {

/// <summary>
/// 将批次3场景拓扑、诊断与加速摘要输出到日志系统。
/// </summary>
/// <param name="scene">待输出摘要的场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch3Summary(const Scene& scene, Logger& logger)
{
    std::ostringstream topologyStream;
    topologyStream << "Batch3Topology: faces=" << scene.faces.size()
                   << ", edges=" << scene.edges.size()
                   << ", wedges=" << scene.wedges.size();
    logger.Log(LogLevel::Info, "Module2", topologyStream.str());

    std::ostringstream diagnosticsStream;
    diagnosticsStream << "SceneDiagnostics: degenerate_faces=" << scene.diagnostics.degenerate_faces.size()
                      << ", non_manifold_edges=" << scene.diagnostics.non_manifold_edges.size()
                      << ", duplicated_faces=" << scene.diagnostics.duplicated_faces.size()
                      << ", flipped_normal_faces=" << scene.diagnostics.flipped_normal_faces.size()
                      << ", faces_missing_dual_side_material=" << scene.diagnostics.faces_missing_dual_side_material.size()
                      << ", passed=" << (scene.diagnostics.passed ? "true" : "false");
    logger.Log(LogLevel::Info, "Module2", diagnosticsStream.str());

    std::ostringstream bvhStream;
    bvhStream << "SceneAcceleration: bvh_nodes=" << scene.acceleration.face_acceleration.bvh_node_count
              << ", leaf_nodes=" << scene.acceleration.face_acceleration.leaf_node_count
              << ", bvh_max_depth=" << scene.acceleration.diagnostics.face_bvh_max_depth
              << ", avg_leaf_faces=" << scene.acceleration.diagnostics.face_bvh_average_leaf_faces
              << ", wedge_query_records=" << scene.acceleration.wedge_acceleration.wedge_query_records.size()
              << ", brute_force_validation_passed="
              << (scene.acceleration.diagnostics.brute_force_validation_passed ? "true" : "false");
    logger.Log(LogLevel::Info, "Module2", bvhStream.str());

    for (const std::string& warning : scene.acceleration.diagnostics.warnings)
    {
        logger.Log(LogLevel::Warn, "Module2", "AccelerationWarning: " + warning);
    }

    for (const std::string& error : scene.acceleration.diagnostics.errors)
    {
        logger.Log(LogLevel::Error, "Module2", "AccelerationError: " + error);
    }
}

} // namespace rt
