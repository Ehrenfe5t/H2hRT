// 文件目标：
// - 定义模块2批次3使用的场景绕射楔边结构。
//
// 主要功能：
// - 保存由拓扑边恢复出的楔边几何、两侧面与材料语义；
// - 为后续模块4候选绕射发现提供稳定真源；
// - 为加速记录与诊断输出提供可追溯字段。

#pragma once

#include "Face.h"

#include <cstdint>

namespace rt {

/// <summary>
/// v9 D-6: 楔边凸性分类
/// </summary>
enum class WedgeConvexity {
    Unknown = 0,   // 无法判断
    Convex  = 1,   // 凸边 (外角>180°, n>1)
    Concave = 2,   // 凹边 (外角<180°, n<1)
    Boundary = 3   // 边界边 (仅一个相邻面)
};

/// <summary>
/// 楔边传播标志位定义。
/// </summary>
enum WedgeFlags : std::uint32_t {
    WedgeFlagNone = 0,
    WedgeFlagDiffractable = 1u << 0,
    WedgeFlagNonManifoldSource = 1u << 1,
    WedgeFlagCoplanarRejected = 1u << 2,
    WedgeFlagAngleFiltered = 1u << 3
};

/// <summary>
/// 场景绕射楔边结构。
/// </summary>
struct Wedge {
    int wedge_id = -1;
    int source_edge_id = -1;

    int positive_face_id = -1;
    int negative_face_id = -1;
    int zero_face_id = -1;          // v9 D-6: UTD φ/φ' 参考面 (默认=positive_face_id)

    Point3 center_point;
    Point3 segment_start;
    Point3 segment_end;
    Vec3 direction;

    double length = 0.0;
    double wedge_angle_deg = 0.0;    // 外角，UTD wedge index n = exterior_angle / 180°
    double dihedral_angle_deg = 0.0;

    std::string positive_material_name;
    std::string negative_material_name;

    bool diffractable = false;
    bool from_non_manifold_source = false;
    bool valid_for_utd = false;     // v9 D-6: 是否满足UTD基本条件
    WedgeConvexity convexity = WedgeConvexity::Unknown; // v9 D-6
    std::uint32_t wedge_flags = WedgeFlagNone;
    AABB bounds;
};

} // namespace rt
