// 文件目标：
// - 定义模块2批次3使用的场景拓扑边结构。
//
// 主要功能：
// - 表达唯一边键、端点、相邻面与边级诊断状态；
// - 为楔边构建与面拓扑回查提供中间层；
// - 显式标注非流形、边界边、共面边等批次3关心语义。

#pragma once

#include "Face.h"

namespace rt {

/// <summary>
/// 场景拓扑边结构。
/// </summary>
struct Edge {
    int edge_id = -1;
    int vertex_index0 = -1;
    int vertex_index1 = -1;

    int face_id0 = -1;
    int face_id1 = -1;

    Vec3 start;
    Vec3 end;
    Vec3 direction;
    Vec3 midpoint;

    double length = 0.0;
    double dihedral_angle_deg = 0.0;

    bool is_boundary = false;
    bool is_non_manifold = false;
    bool is_coplanar = false;
    bool supports_wedge = false;
};

} // namespace rt
