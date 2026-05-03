// 文件目标：
// - 实现模块5批次7的发射场初始化逻辑。
//
// 主要功能：
// - 设定波长、初始复振幅和默认极化占位；
// - 记录当前频率和基础介质状态；
// - 为路径级电磁主链提供统一初值。

#include "InitializeTxField.h"

namespace rt {

/// <summary>
/// 初始化路径级发射场状态。
/// </summary>
/// <param name="input">电磁求解输入。</param>
/// <param name="field">待写入的场状态累积器。</param>
/// <returns>true 表示初始化成功；false 表示失败。</returns>
bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field)
{
    if (input.config == nullptr)
    {
        return false;
    }
    field.frequency_hz = input.config->em_solver.frequency_hz;
    if (field.frequency_hz <= 0.0)
    {
        return false;
    }
    field.wavelength_m = 299792458.0 / field.frequency_hz;
    field.total_length_m = 0.0;
    field.delay_s = 0.0;
    field.phase_rad = 0.0;
    field.amplitude_real = 1.0;
    field.amplitude_imag = 0.0;
    field.power_linear = 1.0;
    field.current_medium_id = 0;
    field.polarization_vector.x = 1.0;
    field.polarization_vector.y = 0.0;
    field.polarization_vector.z = 0.0;
    field.valid = true;
    return true;
}

} // namespace rt
