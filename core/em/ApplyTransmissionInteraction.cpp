// 文件目标：
// - 实现模块5批次7的透射交互更新逻辑。
//
// 主要功能：
// - 使用第一版简化透射系数更新复振幅；
// - 维持极化占位向量连续性；
// - 为后续模块5严格介质参数查询预留接入位置。

#include "ApplyTransmissionInteraction.h"

namespace rt {

/// <summary>
/// 对透射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前透射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node)
{
    if (!field.valid || !node.valid)
    {
        return false;
    }
    const double coefficient = 0.5;
    field.amplitude_real *= coefficient;
    field.amplitude_imag *= coefficient;
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    return true;
}

} // namespace rt
