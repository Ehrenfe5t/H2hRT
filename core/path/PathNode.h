// 文件目标：
// - 定义模块4几何路径节点结构。
//
// 主要功能：
// - 承载每个路径节点的交互类型、几何点与交互对象引用；
// - 为 SBR P2P tracing history and GeometricPath output提供统一节点格式；
// - 为后续模块5消费路径级几何结果保留基础字段。

#pragma once

#include "InteractionType.h"
#include "../scene/Face.h"

namespace rt {

/// <summary>
/// v9 D-4: 绕射几何诊断字段 — 仅绕射节点有效
/// </summary>
struct DiffractionDiagnostics {
    double edge_parameter_t = 0.0;    // Fermat最优点在边上的参数 [0,1]
    double s1 = 0.0;                   // Tx到绕射点距离
    double s2 = 0.0;                   // 绕射点到Rx距离
    double keller_residual = 0.0;     // |cosβ_tx - cosβ_rx| (Keller cone残差)
    bool fermat_endpoint_warning = false; // 最优点接近边端点
    bool visibility_from_source = false;
    bool visibility_to_rx = false;
};

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
    // v9 Stage2: Snell诊断 (仅透射节点)
    double snell_residual = 0.0;           // |n1·sinθ_i - n2·sinθ_t|
    double snell_theta_i_rad = 0.0;        // 入射角
    double snell_theta_t_rad = 0.0;        // 出射角
    bool snell_tir = false;                // 全内反射标志
    DiffractionDiagnostics diffraction_diag; // v9 D-4: 绕射诊断 (仅绕射节点)
};

} // namespace rt
