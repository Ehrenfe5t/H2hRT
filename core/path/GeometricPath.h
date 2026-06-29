// 文件目标：
// - 定义模块4输出给模块5的稳定几何路径结构。
//
// 主要功能：
// - 保存稳定节点序列、总长度、路径签名与基础调试信息；
// - 表达已成立的几何真实路径，而不是中间运行态；
// - 为模块5严格电磁求解提供直接输入容器。

#pragma once

#include "PathNode.h"

#include <cstdint>
#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 稳定几何路径结构。
/// </summary>
struct GeometricPath {
    int path_id = -1;
    std::vector<PathNode> nodes;
    double total_length = 0.0;
    bool is_los = false;
    bool contains_transmission = false;
    uint64_t path_signature = 0;
    // Launch-power fraction represented by this path. Ray-sampled SBR paths
    // use 1/ray_count; deterministic LOS/analytical paths keep unit weight.
    double sampling_weight = 1.0;
    // Number of launch rays that discovered this deterministic path topology.
    // This is a convergence diagnostic and must not scale specular-path power.
    int candidate_support_count = 1;
    bool geometry_refined = false;
    int refinement_iterations = 0;
    std::string refinement_method;
    double geometry_residual = 0.0;
    double reflection_residual_m = 0.0;
    double max_reflection_residual = 0.0;
    double max_snell_residual = 0.0;
    double max_keller_residual = 0.0;
    std::string residual_reject_reason;
    bool valid = false;
};

/// <summary>
/// 几何路径集合结构。
/// </summary>
struct GeometricPathSet {
    std::vector<GeometricPath> paths;
};

} // namespace rt
