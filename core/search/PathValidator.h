// v10 Iter6: 路径验证器 — 完整几何验证 + 硬约束检查

#pragma once

#include "../path/PathNode.h"
#include "../scene/Scene.h"

#include <vector>

namespace rt {

class SceneQuery;

struct PathValidationResult {
    bool valid = false;
    std::vector<bool> point_in_face;       // 交互点是否在面元内
    std::vector<bool> visibility_ok;       // 段可见性
    std::vector<double> snell_residuals;   // 透射Snell残差
    std::vector<double> keller_residuals;  // 绕射Keller残差
    std::vector<double> reflection_residuals; // 反射镜面残差
    std::string failure_reason;
};

class PathValidator {
public:
    PathValidationResult Validate(
        const std::vector<PathNode>& nodes,
        const Point3& tx, const Point3& rx,
        const Scene& scene, const SceneQuery& query) const;
};

} // namespace rt
