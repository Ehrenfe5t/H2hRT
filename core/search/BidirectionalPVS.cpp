// v10 Iter2: 双向PVS收缩器实现

#include "BidirectionalPVS.h"
#include "../query/SceneQuery.h"
#include <algorithm>
#include <cstdio>
#include <iterator>
#include <set>

namespace rt {

// ── 辅助 ──

static void SortUnique(std::vector<int>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

static std::vector<int> SetIntersection(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> result;
    if (a.empty() || b.empty()) return result;
    result.reserve(std::min(a.size(), b.size()));
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    return result;
}

static std::vector<int> SetUnion(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> result;
    result.reserve(a.size() + b.size());
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    return result;
}

// ── Tx/Rx 初始层 ──

CandidateLayer BidirectionalPVSContraction::MakeTxLayer(
    const Point3& tx, const Scene& scene, const SceneQuery& query) const
{
    CandidateLayer layer;
    layer.type = InteractionLayerType::Reflection;

    // 用 BVH 从 Tx 向各方向发射少量采样射线找可见面元
    // 简化: 从 Tx 向 Rx 方向附近采样
    // 实际实现: 迭代所有 reflection_enabled 面元，检查可见性 (限制在附近)
    // 对于初始层，可以放宽为"所有面元"——后续 PVS 扩张会自然过滤

    // 更实际的方案: 用 PVS 的半球采样逻辑
    // 这里先返回所有 reflection_enabled 面元 (备选), PVS 扩张时自然过滤
    for (int i = 0; i < static_cast<int>(scene.faces.size()); ++i) {
        const Face& f = scene.faces[i];
        if (f.degenerate) continue;
        if (f.reflection_enabled || f.transmission_enabled) {
            layer.face_ids.push_back(i);
        }
    }
    SortUnique(layer.face_ids);
    return layer;
}

CandidateLayer BidirectionalPVSContraction::MakeRxLayer(
    const Point3& rx, const Scene& scene, const SceneQuery& query) const
{
    // 与 MakeTxLayer 对称
    return MakeTxLayer(rx, scene, query);
}

// ── PVS 扩张 ──

CandidateLayer BidirectionalPVSContraction::ExpandForward(
    const CandidateLayer& current, const FacePVS& pvs) const
{
    CandidateLayer next;
    next.type = current.type;

    std::set<int> seen;
    for (int fid : current.face_ids) {
        const auto& visible = pvs.GetVisibleFaces(fid);
        for (int vf : visible) {
            if (seen.insert(vf).second) {
                next.face_ids.push_back(vf);
            }
        }
    }
    SortUnique(next.face_ids);

    // 楔边扩张: 如果当前层有关联楔边, 通过 EdgeAdjacency 扩展
    for (int wid : current.wedge_ids) {
        // EdgeAdjacency 暂未在 CandidateLayer 中接入, 留 Iter 6 完善
    }

    return next;
}

CandidateLayer BidirectionalPVSContraction::ExpandBackward(
    const CandidateLayer& current, const FacePVS& pvs) const
{
    // 使用 reverse_pvs: "谁能看到当前层的面"
    CandidateLayer next;
    next.type = current.type;

    std::set<int> seen;
    for (int fid : current.face_ids) {
        if (fid < 0 || fid >= static_cast<int>(pvs.reverse_pvs.size())) continue;
        for (int rf : pvs.reverse_pvs[fid]) {
            if (seen.insert(rf).second) {
                next.face_ids.push_back(rf);
            }
        }
    }
    SortUnique(next.face_ids);
    return next;
}

// ── 类型过滤 ──

void BidirectionalPVSContraction::FilterByType(
    CandidateLayer& layer, InteractionLayerType type, const Scene& scene) const
{
    std::vector<int> filtered;
    for (int fid : layer.face_ids) {
        if (fid < 0 || fid >= static_cast<int>(scene.faces.size())) continue;
        const Face& f = scene.faces[fid];
        if (f.degenerate) continue;
        switch (type) {
            case InteractionLayerType::Reflection:
                if (f.reflection_enabled) filtered.push_back(fid);
                break;
            case InteractionLayerType::Transmission:
                if (f.transmission_enabled && f.dual_side_material_resolved) filtered.push_back(fid);
                break;
            case InteractionLayerType::Diffraction:
                // 楔边过滤 — 在 Expand 阶段通过 wedge_ids 处理
                break;
        }
    }
    layer.face_ids = std::move(filtered);
}

// ── 交集 ──

CandidateLayer BidirectionalPVSContraction::Intersect(
    const CandidateLayer& a, const CandidateLayer& b) const
{
    CandidateLayer result;
    result.type = a.type;
    result.face_ids = SetIntersection(a.face_ids, b.face_ids);
    result.wedge_ids = SetIntersection(a.wedge_ids, b.wedge_ids);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 主入口: 双向 PVS 收缩
// ═══════════════════════════════════════════════════════════

BidirectionalPVSResult BidirectionalPVSContraction::Contract(
    const Point3& tx,
    const Point3& rx,
    const PathSearchConfig& config,
    const Scene& scene,
    const SceneQuery& query) const
{
    BidirectionalPVSResult result;
    const int maxDepth = config.max_path_depth;
    if (maxDepth < 1) return result;

    const auto& pvs = scene.visibility.face_pvs;
    if (!pvs.valid) {
        std::fprintf(stderr, "[BidirectionalPVS] FacePVS not valid — cannot contract\n");
        return result;
    }

    int N = maxDepth;

    // ── 初始化 ──
    result.forward_layers.resize(N + 1);
    result.backward_layers.resize(N + 1);
    result.midpoints.resize(N + 1);

    // Forward[0] = Tx 方向初始候选
    result.forward_layers[0] = MakeTxLayer(tx, scene, query);

    // Backward[0] = Rx 方向初始候选
    result.backward_layers[0] = MakeRxLayer(rx, scene, query);

    // ── 对称扩张 ──
    for (int d = 1; d <= N; ++d) {
        // 正向: 用 pvs_faces 扩张
        result.forward_layers[d] = ExpandForward(result.forward_layers[d - 1], pvs);

        // 反向: 用 reverse_pvs 扩张
        result.backward_layers[d] = ExpandBackward(result.backward_layers[d - 1], pvs);
    }

    // ── 中层交集 ──
    for (int k = 0; k <= N; ++k) {
        result.midpoints[k] = Intersect(result.forward_layers[k], result.backward_layers[N - k]);
    }

    // ── 统计 ──
    for (const auto& layer : result.forward_layers) {
        result.total_face_candidates += static_cast<int>(layer.face_ids.size());
        result.total_wedge_candidates += static_cast<int>(layer.wedge_ids.size());
    }

    result.valid = true;
    return result;
}

} // namespace rt
