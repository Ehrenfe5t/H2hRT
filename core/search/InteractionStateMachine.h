// v10 Iter3: 交互类型状态机 — 枚举所有合法交互序列
// 给定 max_R/T/D + max_depth, 生成所有物理上可能的交互类型组合

#pragma once

#include "../common/config/AppConfig.h"
#include "../path/InteractionType.h"

#include <cstdint>
#include <string>
#include <vector>

namespace rt {

/// 单个交互步骤的类型描述
struct InteractionStep {
    InteractionType type;       // Reflection / Transmission / Diffraction
    int remaining_R;            // 执行此步后剩余的反射配额
    int remaining_T;            // 剩余透射配额
    int remaining_D;            // 剩余绕射配额
    bool can_close_to_rx;       // 此步骤后是否可直接闭合到 Rx (总是 true)
};

/// 完整的交互类型序列 (不含具体面元, 仅类型模式)
struct InteractionSequence {
    std::vector<InteractionStep> steps;  // 从 Tx 出发的交互序列
    int total_depth;                      // 总交互次数
    uint64_t type_signature;              // 序列类型哈希 (用于快速查找)
    bool mixed;                           // 是否包含多种交互类型 (>1 种)
    bool contains_transmission;           // 含透射
    bool contains_diffraction;            // 含绕射

    /// 统计各类交互次数
    int CountR() const;
    int CountT() const;
    int CountD() const;
};

/// 交互类型状态机 — 穷举合法序列
class InteractionStateMachine {
public:
    InteractionStateMachine() = default;

    /// 枚举所有合法交互类型序列
    /// @param config 搜索配置 (含 max_R/T/D, max_depth)
    /// @return 合法序列列表 (典型: 800-1500 for R≤4,T≤2,D≤1)
    std::vector<InteractionSequence> Enumerate(const PathSearchConfig& config) const;

    /// 计算序列的类型签名 (用于 O(1) 查表)
    static uint64_t ComputeTypeSignature(const std::vector<InteractionType>& types);

private:
    void Generate(
        int remaining_R, int remaining_T, int remaining_D,
        int max_depth, int current_depth,
        InteractionType prev_type,
        std::vector<InteractionStep>& current,
        std::vector<InteractionSequence>& output,
        const PathSearchConfig& config) const;

    /// 过滤非法转移 (e.g., T→T 在室内罕见, D→D 暂不支持)
    bool IsValidTransition(InteractionType prev, InteractionType next) const;

    /// 序列是否已包含所有必要类型 (用于去重)
    uint64_t HashSequence(const std::vector<InteractionType>& types) const;
};

} // namespace rt
