// 文件目标：
// - 定义模块2在批次3阶段统一使用的几何基础结构与三角面结构。
//
// 主要功能：
// - 提供 Vec3、AABB、LocalFrame、传播标志位等基础几何类型；
// - 承载面元顶点索引、法向、双侧材质与拓扑关联；
// - 为批次3的 Edge/Wedge/BVH/QueryRecord 构建提供稳定字段入口。

#pragma once

#include <cstdint>
#include <string>

// Vec3/Point3 定义移至 core/common/math/Vec3.h
#include "../common/math/Vec3.h"

namespace rt {

/// <summary>
/// 轴对齐包围盒结构。
/// </summary>
struct AABB {
    Vec3 min;
    Vec3 max;
    bool valid = false;
};

/// <summary>
/// 局部坐标基结构。
/// </summary>
struct LocalFrame {
    Vec3 tangent;
    Vec3 bitangent;
    Vec3 normal;
    bool valid = false;
};

/// <summary>
/// 面传播标志位定义。
/// </summary>
enum FacePropagationFlags : std::uint32_t {
    FacePropagationNone = 0,
    FacePropagationReflect = 1u << 0,
    FacePropagationTransmit = 1u << 1,
    FacePropagationDiffractionBoundary = 1u << 2,
    FacePropagationScatter = 1u << 3,
    FacePropagationDualSideResolved = 1u << 4
};

/// <summary>
/// 场景三角面元结构。
/// </summary>
struct Face {
    int face_id = -1;
    int object_id = -1;
    int vertex_index0 = -1;
    int vertex_index1 = -1;
    int vertex_index2 = -1;
    int normal_index = -1;

    Vec3 normal;
    Vec3 centroid;
    AABB bounds;
    double area = 0.0;
    LocalFrame local_frame;

    std::string object_name;
    std::string object_type;
    std::string surface_material_name;
    int surface_material_id = -1;
    std::string front_material_name;
    int front_material_id = -1;
    int front_medium_id = -1;
    std::string back_material_name;
    int back_material_id = -1;
    int back_medium_id = -1;
    std::string normal_rule_tag;

    bool dual_side_material_resolved = false;
    bool reflection_enabled = true;
    bool transmission_enabled = false;
    bool diffraction_candidate_enabled = false;
    bool transmission_semantic_complete = false;
    bool degenerate = false;

    std::uint32_t propagation_flags = FacePropagationNone;

    int adjacent_edge_id0 = -1;
    int adjacent_edge_id1 = -1;
    int adjacent_edge_id2 = -1;
};

} // namespace rt
