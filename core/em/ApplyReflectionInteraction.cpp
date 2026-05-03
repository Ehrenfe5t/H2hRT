// 文件目标：
// - 实现模块5批次7的反射交互更新逻辑。
//
// 主要功能：
// - 使用第一版简化反射系数更新复振幅；
// - 更新极化占位向量；
// - 为后续更严格 Fresnel 反射系数实现预留位置。

#include "ApplyReflectionInteraction.h"

namespace rt {

/// <summary>
/// 对反射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前反射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node)
{
    if (!field.valid || !node.valid)
    {
        return false;
    }
    const double coefficient = -0.7;
    field.amplitude_real *= coefficient;
    field.amplitude_imag *= coefficient;
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    field.polarization_vector = node.surface_normal;
    return true;
}

} // namespace rt
