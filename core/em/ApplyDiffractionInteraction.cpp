// 文件目标：
// - 实现模块5批次7的绕射交互更新逻辑。
//
// 主要功能：
// - 使用第一版“几何与频率相关”的绕射系数更新复振幅；
// - 保持路径级主链可运行；
// - 为后续更严格 UTD 模型预留替换位置。

#include "ApplyDiffractionInteraction.h"

#include <cmath>

namespace rt {

namespace {

double WedgeStrength(int wedgeId)
{
    if (wedgeId < 0)
    {
        return 0.85;
    }
    return 0.72 + 0.03 * static_cast<double>(wedgeId % 5);
}

} // namespace

/// <summary>
/// 对绕射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前绕射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node)
{
    if (!field.valid || !node.valid)
    {
        return false;
    }
    // A4-2 的 diffraction 升级目标：
    // 1) 不再固定乘子；
    // 2) 至少让 wedge 语义 / 路径结构差异 / 频率变化对结果可见；
    // 3) 保持模块5主链不变。
    field.amplitude_real *= WedgeStrength(node.wedge_id) * (0.65 + 0.15 * std::min(1.0, field.total_length_m / 10.0));
    field.amplitude_imag *= WedgeStrength(node.wedge_id) * (0.65 + 0.15 * std::min(1.0, field.total_length_m / 10.0));
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    return true;
}

} // namespace rt
