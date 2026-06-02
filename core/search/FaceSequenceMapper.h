// v10: 面元序列映射器 — 类型序列 × PVS层 → 具体面元序列
// 核心: 对每种交互序列类型, 从双向 PVS 收缩的层中提取可行面元/楔边组合

#pragma once

#include "InteractionStateMachine.h"
#include "BidirectionalPVS.h"
#include "../scene/Scene.h"

#include <vector>
#include <cstdint>

namespace rt {

/// 具体的面元/楔边交互序列
struct FaceInteractionSequence {
    std::vector<int> face_ids;       // 面元 ID (反射/透射)
    std::vector<int> wedge_ids;      // 楔边 ID (绕射, -1 表示非绕射)
    std::vector<InteractionType> types;  // 对应交互类型
    uint64_t type_sig;               // 类型序列签名 (用于分组)

    /// 是否全为反射 (可用级联镜像法解析)
    bool IsPureReflection() const;
    /// 是否含绕射
    bool HasDiffraction() const;
    /// 总交互次数
    int Depth() const { return static_cast<int>(types.size()); }
};

/// 面元序列映射器 — 从 PVS 层中为每种交互类型序列生成具体面元组合
class FaceSequenceMapper {
public:
    FaceSequenceMapper() = default;

    /// 主入口: 类型序列 + PVS 收缩结果 → 面元序列列表
    /// @param type_seqs 交互类型序列 (来自 InteractionStateMachine)
    /// @param pvs_result 双向 PVS 收缩结果
    /// @param scene 场景对象 (用于 face 属性查询)
    /// @param max_combinations 每个类型序列的最大面元组合数 (防爆炸)
    std::vector<FaceInteractionSequence> Map(
        const std::vector<InteractionSequence>& type_seqs,
        const BidirectionalPVSResult& pvs_result,
        const Scene& scene,
        int max_combinations = 200) const;

private:
    /// 为单个类型序列映射面元组合
    void MapOneSequence(
        const InteractionSequence& seq,
        const BidirectionalPVSResult& pvs,
        const Scene& scene,
        int max_combinations,
        std::vector<FaceInteractionSequence>& output) const;

    /// 从 PVS 层中取某类型某层的候选面元
    std::vector<int> GetLayerCandidates(
        InteractionType type, int depth,
        const BidirectionalPVSResult& pvs,
        const Scene& scene) const;

    /// 笛卡尔积展开: 对各层候选取组合 (限制最大数)
    void ExpandCombinations(
        const std::vector<std::vector<int>>& layer_candidates,
        const InteractionSequence& seq,
        int max_combinations,
        std::vector<FaceInteractionSequence>& output) const;
};

} // namespace rt
