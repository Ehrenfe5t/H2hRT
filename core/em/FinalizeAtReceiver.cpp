// 文件目标：
// - 实现模块5批次7的接收端结果收敛逻辑。
//
// 主要功能：
// - 从 FieldAccumulator 生成路径级 EMPathResult；
// - 保留当前批次要求的 delay / phase / complex amplitude / power；
// - 为后续 CIR/PDP/APS 汇总提供路径级结果真源。

#include "FinalizeAtReceiver.h"

#include <cmath>

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
    result.free_space_amplitude_scale = field.free_space_amplitude_scale;
    result.free_space_power_scale = field.free_space_power_scale;
    result.wavelength_m = field.wavelength_m;
    result.polarization_magnitude = std::sqrt(field.polarization_vector.x * field.polarization_vector.x + field.polarization_vector.y * field.polarization_vector.y + field.polarization_vector.z * field.polarization_vector.z);
    result.free_space_loss_db = (field.free_space_power_scale > 0.0) ? (-10.0 * std::log10(field.free_space_power_scale)) : 0.0;
    result.tx_antenna_id = field.tx_antenna_id;
    result.tx_antenna_source_type = field.tx_antenna_source_type;
    result.rx_antenna_id = field.rx_antenna_id;
    result.rx_antenna_source_type = field.rx_antenna_source_type;
    result.polarization_vector = field.polarization_vector;
    result.is_los = path.is_los;
    result.source_path_signature = path.path_signature;
    result.source_tag = "search_engine_real_output";
    result.contains_transmission = path.contains_transmission;
    result.transmission_semantic_consumed = field.transmission_semantic_consumed;
    result.first_transmission_medium_in_id = field.last_transmission_medium_in_id;
    result.first_transmission_medium_out_id = field.last_transmission_medium_out_id;
    result.last_transmission_medium_in_id = field.last_transmission_medium_in_id;
    result.last_transmission_medium_out_id = field.last_transmission_medium_out_id;
    return result;
}

} // namespace rt
