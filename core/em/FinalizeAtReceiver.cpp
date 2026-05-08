// 接收端收敛: 将累积场状态打包为EMPathResult。
// C1(v4): FSPL用Friis标准形式 λ/(4πd) 显式表达。远场近似: d > 2D²/λ时成立(2.4GHz下~4m)。
//         介质衰减exp(-α_tot) + Rx天线增益(若方向图已加载)一并应用。

#include "FinalizeAtReceiver.h"
#include "../common/math/Vec3.h"
#include "../common/math/MathConstants.h"
#include "../common/math/CoordinateFrame.h"
#include "../antenna/AntennaFactory.h"
#include <cmath>

namespace rt {

EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path, const EMSolverInput& input)
{
    EMPathResult result;
    result.path_id = path.path_id;
    result.valid = field.valid && path.valid;
    if (!result.valid) return result;

    result.total_length_m = field.total_length_m;
    result.delay_s = field.delay_s;
    result.phase_rad = field.phase_rad;
    result.wavelength_m = field.wavelength_m;

    // ── Friis自由空间场衰减: A_fspl = λ / (4πd) ──
    // 功率衰减: L_fspl = (λ/(4πd))² = 1 / ((4πd/λ)²)
    // 适用条件: d > 2D²/λ (远场), 对于2.4GHz(λ≈0.125m)和0.5m天线, 远场边界≈4m
    double fsplAmp = field.wavelength_m / (4.0 * kPi * field.total_length_m);
    result.free_space_amplitude_scale = fsplAmp;
    result.free_space_power_scale = fsplAmp * fsplAmp;
    result.free_space_loss_db = (result.free_space_power_scale > 0.0)
        ? (-10.0 * std::log10(result.free_space_power_scale)) : 0.0;

    // 介质衰减: exp(-Σ α_i·d_i), α_i=各段介质衰减常数(Np/m)
    double mediaLoss = std::exp(-field.media_attenuation_np);

    // Rx天线增益(若方向图已加载)
    double rxGainLin = 1.0;
    if (input.config) {
        const Point3 rxPos = !path.nodes.empty() ? path.nodes.back().point : Point3{};
        AntennaModel rxAnt = BuildRxAntennaModel(*input.config, rxPos, "rx-finalize");
        if (rxAnt.pattern.loaded && path.nodes.size() >= 2) {
            Vec3 incDir = Normalize(Subtract(path.nodes[path.nodes.size()-1].point,
                                              path.nodes[path.nodes.size()-2].point));
            double thetaD, phiD;
            CartesianToSpherical(incDir, thetaD, phiD);
            thetaD *= 180.0 / kPi; phiD *= 180.0 / kPi;
            double gainDBi = rxAnt.pattern.QueryGainDBi(thetaD, phiD);
            rxGainLin = std::pow(10.0, gainDBi / 10.0);
        }
    }

    double totalScale = fsplAmp * mediaLoss * std::sqrt(rxGainLin);
    result.amplitude_real = field.amplitude_real * totalScale;
    result.amplitude_imag = field.amplitude_imag * totalScale;
    result.power_linear = result.amplitude_real * result.amplitude_real
                        + result.amplitude_imag * result.amplitude_imag;

    // 极化信息 (v5 Jones: 实部+虚部)
    result.polarization_vector = field.polarization_vector;
    result.polarization_imag = field.polarization_imag;
    result.polarization_magnitude = std::sqrt(Dot(field.polarization_vector, field.polarization_vector)
                                            + Dot(field.polarization_imag, field.polarization_imag));

    // 元数据拷贝
    result.tx_antenna_id = field.tx_antenna_id;
    result.tx_antenna_source_type = field.tx_antenna_source_type;
    result.rx_antenna_id = field.rx_antenna_id;
    result.rx_antenna_source_type = field.rx_antenna_source_type;
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

EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path) {
    EMSolverInput dummy; dummy.config = nullptr;
    return FinalizeAtReceiver(field, path, dummy);
}

} // namespace rt
