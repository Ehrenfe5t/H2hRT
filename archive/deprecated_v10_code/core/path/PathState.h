// 文件目标：
// - 定义模块4统一路径状态机使用的运行时状态对象。
//
// 主要功能：
// - 保存当前点、方向、介质、预算和运行历史；
// - 为 SceneQuery 调用派生局部查询上下文提供真源；
// - 区分中间搜索态与最终 GeometricPath 结果态。

#pragma once

#include "InteractionType.h"
#include "PathNode.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 模块4运行时路径状态对象。
/// </summary>
struct PathState {
    Point3 current_point;
    Vec3 current_direction;
    int current_medium_id = -1;

    InteractionType last_interaction_type = InteractionType::None;
    int last_interaction_object_id = -1;
    int last_hit_face_id = -1;
    int last_hit_wedge_id = -1;
    int last_medium_in_id = -1;
    int last_medium_out_id = -1;
    int last_front_medium_id = -1;
    int last_back_medium_id = -1;
    Vec3 last_interaction_normal;
    bool last_hit_front_side = true;
    bool last_transmission_semantic_complete = false;

    std::vector<PathNode> traversed_nodes;
    double accumulated_length = 0.0;
    int path_depth = 0;
    int interaction_count = 0;

    int remaining_total_expansions = 0;
    int remaining_reflections = 0;
    int remaining_transmissions = 0;
    int remaining_diffractions = 0;
    int remaining_scatterings = 0;

    int ignored_face_id = -1;
    int ignored_object_id = -1;

    bool allow_reflection = true;
    bool allow_transmission = true;
    bool allow_diffraction = true;
    bool allow_scattering = false;

    int mechanism_switch_count = 0;
    int consecutive_same_interaction_count = 0;
    bool mixed_path_enabled = false;
    bool has_reflection = false;
    bool has_transmission = false;
    bool has_diffraction = false;
    bool clipped_by_control_rules = false;

    uint64_t state_signature = 0;
    bool valid = false;
};

} // namespace rt
