// v10 Iter2: 双向可见性收缩 — Tx/Rx 对称扩张 + 层交集
// 核心: 用 FacePVS + reverse_pvs 双向扩大候选集, 交集收缩

#pragma once

#include "../common/config/AppConfig.h"
#include "../scene/Scene.h"
#include "../scene/SceneVisibilityData.h"

#include <vector>
#include <string>

namespace rt {

class SceneQuery;

/// 交互层类型
enum class InteractionLayerType {
    Reflection,
    Transmission,
    Diffraction
};

/// 一层候选 — 对应路径中的一个交互位置
struct CandidateLayer {
    InteractionLayerType type = InteractionLayerType::Reflection;
    std::vector<int> face_ids;      // 候选面元 (反射/透射)
    std::vector<int> wedge_ids;     // 候选楔边 (绕射)
    size_t size() const { return face_ids.size() + wedge_ids.size(); }
};

/// 双向 PVS 收缩结果
struct BidirectionalPVSResult {
    bool valid = false;
    std::vector<CandidateLayer> forward_layers;   // Forward[0]=Tx层, Forward[1]=第1跳候选
    std::vector<CandidateLayer> backward_layers;   // Backward[0]=Rx层, Backward[1]=反向第1跳
    std::vector<CandidateLayer> midpoints;         // 中层交集
    int total_face_candidates = 0;
    int total_wedge_candidates = 0;
    double build_time_ms = 0.0;
};

/// 双向 PVS 收缩器
class BidirectionalPVSContraction {
public:
    BidirectionalPVSContraction() = default;

    /// 从 Tx/Rx 两端对称扩张候选层
    BidirectionalPVSResult Contract(
        const Point3& tx,
        const Point3& rx,
        const PathSearchConfig& config,
        const Scene& scene,
        const SceneQuery& query) const;

private:
    /// Tx 初始层: 从 Tx 位置可见的面元 (BVH 半球采样或 PVS 查找)
    CandidateLayer MakeTxLayer(const Point3& tx, const Scene& scene, const SceneQuery& query) const;

    /// Rx 初始层: 从 Rx 位置可见的面元
    CandidateLayer MakeRxLayer(const Point3& rx, const Scene& scene, const SceneQuery& query) const;

    /// 单步正向扩张: 从当前层扩展到下一层
    CandidateLayer ExpandForward(
        const CandidateLayer& current,
        const FacePVS& pvs) const;

    /// 单步反向扩张: 用 reverse_pvs 反向扩展
    CandidateLayer ExpandBackward(
        const CandidateLayer& current,
        const FacePVS& pvs) const;

    /// 按交互类型过滤候选面元
    void FilterByType(CandidateLayer& layer, InteractionLayerType type, const Scene& scene) const;

    /// 取两个层的交集
    CandidateLayer Intersect(const CandidateLayer& a, const CandidateLayer& b) const;
};

} // namespace rt
