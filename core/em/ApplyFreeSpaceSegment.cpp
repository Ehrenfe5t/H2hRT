// 文件目标：
// - 实现模块5批次7的自由空间段传播逻辑。
//
// 主要功能：
// - 累积总长度、时延和相位；
// - 采用第一版简化自由空间振幅衰减模型；
// - 为路径级交互更新提供统一段传播主干。

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

    const double attenuation = 1.0 / (1.0 + segmentLengthM);
    field.amplitude_real *= attenuation;
    field.amplitude_imag *= attenuation;
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    return true;
}

} // namespace rt
