// 文件目标：
// - 实现模块5批次7的接收端结果收敛逻辑。
//
// 主要功能：
// - 从 FieldAccumulator 生成路径级 EMPathResult；
// - 保留当前批次要求的 delay / phase / complex amplitude / power；
// - 为后续 CIR/PDP/APS 汇总提供路径级结果真源。

#include "FinalizeAtReceiver.h"

namespace rt {

/// <summary>
/// 将路径级场状态收敛为接收端结果。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="path">当前几何路径。</param>
/// <returns>单条路径级电磁结果。</returns>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path)
{
    EMPathResult result;
    result.path_id = path.path_id;
    result.valid = field.valid && path.valid;
    result.total_length_m = field.total_length_m;
    result.delay_s = field.delay_s;
    result.phase_rad = field.phase_rad;
    result.amplitude_real = field.amplitude_real;
    result.amplitude_imag = field.amplitude_imag;
    result.power_linear = field.power_linear;
    result.polarization_vector = field.polarization_vector;
    result.is_los = path.is_los;
    return result;
}

} // namespace rt
