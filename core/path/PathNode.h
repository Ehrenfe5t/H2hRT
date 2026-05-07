// 文件目标：
// - 定义模块4几何路径节点结构。
//
// 主要功能：
// - 承载每个路径节点的交互类型、几何点与交互对象引用；
// - 为 PathState 运行历史和 GeometricPath 稳定输出提供统一节点格式；
// - 为后续模块5消费路径级几何结果保留基础字段。

#pragma once

#include "InteractionType.h"
#include "../scene/Face.h"

namespace rt {

/// <summary>
/// 几何路径节点结构。
/// </summary>
struct PathNode {
    InteractionType interaction_type = InteractionType::None;
    int object_id = -1;
    int face_id = -1;
    int wedge_id = -1;
    int medium_in_id = -1;
    int medium_out_id = -1;
    int front_medium_id = -1;
    int back_medium_id = -1;
    int front_material_id = -1;
    int back_material_id = -1;
    bool entered_from_front_side = true;
    bool transmission_semantic_complete = false;
    Point3 point;
    Vec3 direction;              // outgoing direction (after interaction, toward next node/Rx)
    Vec3 incident_direction;      // C3-A(v4): incoming direction (before interaction). Used for UTD phi' calc.
    Vec3 surface_normal;
    double segment_length_from_previous = 0.0;
    bool valid = false;
};

} // namespace rt
