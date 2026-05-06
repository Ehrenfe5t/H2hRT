// 文件目标：
// - 实现模块5批次7的透射交互更新逻辑。
//
// 主要功能：
// - 使用介质相关的透射系数更新复振幅；
// - 显式消费 medium_in / medium_out / entered_from_front_side 语义；
// - 为后续更严格 Fresnel 透射系数实现预留接入位置。

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
    if (!node.transmission_semantic_complete)
    {
        return false;
    }
    if (node.medium_in_id < 0 || node.medium_out_id < 0)
    {
        return false;
    }
    if (node.medium_in_id == node.medium_out_id)
    {
        return false;
    }

    // A4-2：让 transmission 至少对前后介质与入射侧语义有可见响应。
    field.amplitude_real *=
        (node.entered_from_front_side ? 1.0 : 0.95)
        * (0.45 + 0.5 * (
            ((node.medium_in_id == 0) ? 1.00 : (node.medium_in_id == 1) ? 0.82 : (node.medium_in_id == 2) ? 0.88 : (node.medium_in_id == 3) ? 0.74 : (node.medium_in_id == 4) ? 0.79 : 0.86)
            /
            ((node.medium_out_id == 0) ? 1.00 : (node.medium_out_id == 1) ? 0.82 : (node.medium_out_id == 2) ? 0.88 : (node.medium_out_id == 3) ? 0.74 : (node.medium_out_id == 4) ? 0.79 : 0.86)));

    field.amplitude_imag *=
        (node.entered_from_front_side ? 1.0 : 0.95)
        * (0.45 + 0.5 * (
            ((node.medium_in_id == 0) ? 1.00 : (node.medium_in_id == 1) ? 0.82 : (node.medium_in_id == 2) ? 0.88 : (node.medium_in_id == 3) ? 0.74 : (node.medium_in_id == 4) ? 0.79 : 0.86)
            /
            ((node.medium_out_id == 0) ? 1.00 : (node.medium_out_id == 1) ? 0.82 : (node.medium_out_id == 2) ? 0.88 : (node.medium_out_id == 3) ? 0.74 : (node.medium_out_id == 4) ? 0.79 : 0.86)));

    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    field.last_transmission_medium_in_id = node.medium_in_id;
    field.last_transmission_medium_out_id = node.medium_out_id;
    field.current_medium_id = node.medium_out_id;
    field.transmission_semantic_consumed = true;
    return true;
}

} // namespace rt
