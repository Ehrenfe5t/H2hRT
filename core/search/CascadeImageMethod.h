// v10 Iter1: 级联镜像法 — 多跳纯反射路径的 O(k) 解析解
// 原理: 对 target 连续镜像 → 正向递推求交
// 参考文献: Kim et al. IEEE TAP 2024, Honcharenko IEEE TVT 1992

#pragma once

#include "../common/math/Vec3.h"
#include "../path/PathNode.h"
#include "../scene/Scene.h"

#include <vector>

namespace rt {

/// 级联镜像法结果
struct CascadeImageResult {
    bool valid = false;
    std::vector<Point3> reflection_points;  // [P1, P2, ..., Pm] — 反射点序列
    std::vector<PathNode> nodes;            // 带完整诊断的 PathNode
    double total_length = 0.0;               // 总几何路径长度
    std::string failure_reason;              // 失败原因 (为空表示成功)
};

/// 级联镜像法: 给定 source, target, 和反射面元序列 [f1..fm], 求解反射点
/// @param source 源点 (Tx 或上一个交互点)
/// @param target 目标点 (Rx 或下一个交互点)
/// @param face_ids 反射面元 ID 序列 (按交互顺序)
/// @param scene 场景对象
/// @param query 场景查询接口 (用于 BVH 射线求交)
/// @return 包含反射点序列和完整诊断的结果
CascadeImageResult SolveCascadeReflection(
    const Point3& source,
    const Point3& target,
    const std::vector<int>& face_ids,
    const Scene& scene,
    class SceneQuery& query);

} // namespace rt
