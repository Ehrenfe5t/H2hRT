// 文件目标：
// - 实现模块5批次7的绕射交互更新逻辑。
//
// 主要功能：
// - 使用第一版简化绕射系数更新复振幅；
// - 保持路径级主链可运行；
// - 为后续更严格 UTD 模型预留替换位置。

#include "ApplyDiffractionInteraction.h"

namespace rt {

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
    const double coefficient = 0.3;
    field.amplitude_real *= coefficient;
    field.amplitude_imag *= coefficient;
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    return true;
}

} // namespace rt
