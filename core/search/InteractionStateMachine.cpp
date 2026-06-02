// v10 Iter3: 交互类型状态机实现

#include "InteractionStateMachine.h"
#include <algorithm>

namespace rt {

// ── 内部辅助 (文件作用域) ──

static int CountTypes(const std::vector<InteractionStep>& steps) {
    int c = 0;
    for (auto& s : steps) {
        if (s.type == InteractionType::Reflection) c |= 1;
        if (s.type == InteractionType::Transmission) c |= 2;
        if (s.type == InteractionType::Diffraction) c |= 4;
    }
    return (c & 1 ? 1 : 0) + (c & 2 ? 1 : 0) + (c & 4 ? 1 : 0);
}

static bool HasType(const std::vector<InteractionStep>& steps, InteractionType t) {
    for (auto& s : steps) if (s.type == t) return true;
    return false;
}

// ── InteractionSequence 辅助 ──

int InteractionSequence::CountR() const {
    int c = 0;
    for (auto& s : steps) if (s.type == InteractionType::Reflection) ++c;
    return c;
}
int InteractionSequence::CountT() const {
    int c = 0;
    for (auto& s : steps) if (s.type == InteractionType::Transmission) ++c;
    return c;
}
int InteractionSequence::CountD() const {
    int c = 0;
    for (auto& s : steps) if (s.type == InteractionType::Diffraction) ++c;
    return c;
}

// ── 类型签名 ──

uint64_t InteractionStateMachine::ComputeTypeSignature(const std::vector<InteractionType>& types) {
    uint64_t h = 0;
    for (auto t : types) {
        h = h * 31 + static_cast<uint64_t>(t);
    }
    return h;
}

uint64_t InteractionStateMachine::HashSequence(const std::vector<InteractionType>& types) const {
    return ComputeTypeSignature(types);
}

// ── 转移合法性 ──

bool InteractionStateMachine::IsValidTransition(InteractionType prev, InteractionType next) const {
    // Tx → 任何类型均可
    if (prev == InteractionType::Tx || prev == InteractionType::None) return true;

    // 不允许连续相同类型的透射 (T→T 在室内场景罕见, 且易导致数值退化)
    if (prev == InteractionType::Transmission && next == InteractionType::Transmission) return false;

    // 绕射后暂不支持绕射 (D→D 仅在多楔边场景, 留待后续)
    if (prev == InteractionType::Diffraction && next == InteractionType::Diffraction) return false;

    return true;
}

// ── 递归生成 ──

void InteractionStateMachine::Generate(
    int remaining_R, int remaining_T, int remaining_D,
    int max_depth, int current_depth,
    InteractionType prev_type,
    std::vector<InteractionStep>& current,
    std::vector<InteractionSequence>& output,
    const PathSearchConfig& config) const
{
    // 深度限制
    if (current_depth > max_depth) return;
    if (current_depth > 0 && current_depth <= max_depth) {
        // 当前序列可直接闭合到 Rx
        InteractionSequence seq;
        seq.steps = current;
        seq.total_depth = current_depth;
        seq.mixed = (CountTypes(current) > 1);
        seq.contains_transmission = HasType(current, InteractionType::Transmission);
        seq.contains_diffraction = HasType(current, InteractionType::Diffraction);

        // 计算类型签名
        std::vector<InteractionType> types;
        for (auto& s : current) types.push_back(s.type);
        seq.type_signature = ComputeTypeSignature(types);

        output.push_back(seq);
    }

    if (current_depth >= max_depth) return;

    // 尝试 Reflection
    if (remaining_R > 0 && IsValidTransition(prev_type, InteractionType::Reflection)) {
        InteractionStep step;
        step.type = InteractionType::Reflection;
        step.remaining_R = remaining_R - 1;
        step.remaining_T = remaining_T;
        step.remaining_D = remaining_D;
        step.can_close_to_rx = true;
        current.push_back(step);
        Generate(remaining_R - 1, remaining_T, remaining_D, max_depth, current_depth + 1,
                 InteractionType::Reflection, current, output, config);
        current.pop_back();
    }

    // 尝试 Transmission
    if (remaining_T > 0 && IsValidTransition(prev_type, InteractionType::Transmission)) {
        InteractionStep step;
        step.type = InteractionType::Transmission;
        step.remaining_R = remaining_R;
        step.remaining_T = remaining_T - 1;
        step.remaining_D = remaining_D;
        step.can_close_to_rx = true;
        current.push_back(step);
        Generate(remaining_R, remaining_T - 1, remaining_D, max_depth, current_depth + 1,
                 InteractionType::Transmission, current, output, config);
        current.pop_back();
    }

    // 尝试 Diffraction
    if (remaining_D > 0 && IsValidTransition(prev_type, InteractionType::Diffraction)) {
        InteractionStep step;
        step.type = InteractionType::Diffraction;
        step.remaining_R = remaining_R;
        step.remaining_T = remaining_T;
        step.remaining_D = remaining_D - 1;
        step.can_close_to_rx = true;
        current.push_back(step);
        Generate(remaining_R, remaining_T, remaining_D - 1, max_depth, current_depth + 1,
                 InteractionType::Diffraction, current, output, config);
        current.pop_back();
    }
}

// ── 公共入口 ──

std::vector<InteractionSequence> InteractionStateMachine::Enumerate(const PathSearchConfig& config) const
{
    int maxR = config.max_reflection_count;
    int maxT = config.max_transmission_count;
    int maxD = config.max_diffraction_count;
    int maxDepth = config.max_path_depth;

    std::vector<InteractionSequence> output;
    std::vector<InteractionStep> current;

    // LOS 序列 (深度=0): 无交互, 直达 Rx
    {
        InteractionSequence los_seq;
        los_seq.total_depth = 0;
        los_seq.mixed = false;
        los_seq.contains_transmission = false;
        los_seq.contains_diffraction = false;
        los_seq.type_signature = 0;
        output.push_back(los_seq);
    }

    // 递归生成所有非空序列
    Generate(maxR, maxT, maxD, maxDepth, 0, InteractionType::Tx, current, output, config);

    // 按深度排序
    std::sort(output.begin(), output.end(),
        [](const InteractionSequence& a, const InteractionSequence& b) {
            return a.total_depth < b.total_depth;
        });

    return output;
}

} // namespace rt
