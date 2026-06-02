// v10 Iter6: 路径验证器实现

#include "PathValidator.h"
#include "../query/SceneQuery.h"

namespace rt {

PathValidationResult PathValidator::Validate(
    const std::vector<PathNode>& nodes,
    const Point3& tx, const Point3& rx,
    const Scene& scene, const SceneQuery& query) const
{
    PathValidationResult result;
    int n = static_cast<int>(nodes.size());
    result.point_in_face.resize(n, true);
    result.visibility_ok.resize(n + 1, true);  // n 个交互点 → n+1 个段
    result.snell_residuals.resize(n, 0.0);
    result.keller_residuals.resize(n, 0.0);
    result.reflection_residuals.resize(n, 0.0);

    if (!nodes.empty() && !nodes[0].valid) {
        result.failure_reason = "first node invalid";
        return result;
    }

    // ── 逐段可见性检查 ──
    Point3 prev = tx;
    for (int i = 0; i < n; ++i) {
        VisibilityQueryContext vc;
        if (nodes[i].face_id >= 0) vc.ignored_face_id = nodes[i].face_id;

        bool vis = query.IsVisible(prev, nodes[i].point, vc);
        result.visibility_ok[i] = vis;
        if (!vis) {
            result.failure_reason = "segment " + std::to_string(i) + " visibility blocked";
        }
        prev = nodes[i].point;
    }

    // 最后一段: 最后一个交互点 → Rx
    if (n > 0) {
        VisibilityQueryContext vc;
        if (nodes[n-1].face_id >= 0) vc.ignored_face_id = nodes[n-1].face_id;
        result.visibility_ok[n] = query.IsVisible(nodes[n-1].point, rx, vc);
        if (!result.visibility_ok[n]) {
            result.failure_reason = "final segment visibility blocked";
        }
    }

    // ── Snell残差来自 PathNode 预存值 ──
    for (int i = 0; i < n; ++i) {
        if (nodes[i].interaction_type == InteractionType::Transmission) {
            result.snell_residuals[i] = nodes[i].snell_residual;
            if (nodes[i].snell_tir) {
                result.failure_reason = "TIR transmission node accepted";
                result.valid = false;
                return result;
            }
        }
        if (nodes[i].interaction_type == InteractionType::Diffraction) {
            result.keller_residuals[i] = nodes[i].diffraction_diag.keller_residual;
        }
    }

    result.valid = result.failure_reason.empty();
    return result;
}

} // namespace rt
