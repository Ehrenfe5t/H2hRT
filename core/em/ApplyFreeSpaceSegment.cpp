// 文件目标：
// - 实现模块5批次7的自由空间段传播逻辑。
//
// 主要功能：
// - 累积总长度、时延和相位；
// - 用基于波长与路径长度的自由空间传播基底替换第一版占位衰减；
// - 为后续 interaction 提供可解释的统一传播底座。

#include "ApplyFreeSpaceSegment.h"

#include <cmath>

namespace rt {

/// <summary>
/// 对单段自由空间传播更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="segmentLengthM">当前段长度。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyFreeSpaceSegment(FieldAccumulator& field, double segmentLengthM)
{
    if (!field.valid || segmentLengthM <= 0.0 || field.wavelength_m <= 0.0)
    {
        return false;
    }

    field.total_length_m += segmentLengthM;
    field.delay_s += segmentLengthM / 299792458.0;
    field.phase_rad -= 2.0 * 3.14159265358979323846 * segmentLengthM / field.wavelength_m;

    // A4-1：先把 free-space 从占位衰减替换为单频自由空间传播基底。
    field.amplitude_real *= field.wavelength_m / (4.0 * 3.14159265358979323846 * segmentLengthM);
    field.amplitude_imag *= field.wavelength_m / (4.0 * 3.14159265358979323846 * segmentLengthM);
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    field.free_space_amplitude_scale *= field.wavelength_m / (4.0 * 3.14159265358979323846 * segmentLengthM);
    field.free_space_power_scale = field.free_space_amplitude_scale * field.free_space_amplitude_scale;
    field.last_segment_length_m = segmentLengthM;
    return true;
}

} // namespace rt
