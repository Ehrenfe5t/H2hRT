// 文件目标：
// - 定义模块5批次7的路径级场状态累积器。
//
// 主要功能：
// - 保存逐段传播和逐交互更新中的复场状态；
// - 记录当前介质、路径长度、时延与极化占位信息；
// - 为最终 EMPathResult 组装提供统一中间态。

#pragma once

#include "../path/InteractionType.h"
#include "../scene/Face.h"

namespace rt {

/// <summary>
/// 路径级场状态累积器。
/// </summary>
struct FieldAccumulator {
    double frequency_hz = 0.0;
    double wavelength_m = 0.0;
    double total_length_m = 0.0;
    double delay_s = 0.0;
    double phase_rad = 0.0;
    double amplitude_real = 1.0;
    double amplitude_imag = 0.0;
    double power_linear = 1.0;
    int current_medium_id = -1;
    Vec3 polarization_vector;
    bool valid = false;
};

} // namespace rt
