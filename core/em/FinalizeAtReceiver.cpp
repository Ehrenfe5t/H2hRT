// 接收端收敛: 将累积场状态打包为EMPathResult。
// C1(v4): FSPL用Friis标准形式 λ/(4πd) 显式表达。远场近似: d > 2D²/λ时成立(2.4GHz下~4m)。
//         介质衰减exp(-α_tot) + Rx天线增益(若方向图已加载)一并应用。

#include "FinalizeAtReceiver.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/ComplexVec3.h"
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

    // v9 step2: 零长度保护 — 防止除零导致inf/NaN
    if (field.total_length_m <= kEpsLength) {
        result.valid = false;
        result.amplitude_real = 0.0;
        result.amplitude_imag = 0.0;
        result.power_linear = 0.0;
        result.free_space_loss_db = 0.0;
        return result;
    }

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

    // ── v9 Stage D: 复矢量接收 (共轭匹配 + Rx gain) ──
    if (field.vector_field_valid && path.nodes.size() >= 2) {
        double totalScale = fsplAmp * mediaLoss;
        ComplexVec3 E_inc = Scale(field.electric_field_world, totalScale);

        // Stage D: Rx Jones response — per-angle antenna polarization + gain
        Vec3 rxPolVec = MakeVec3(0.0, 1.0, 0.0); // default: vertical (Y-up)
        Vec3 rxPolImVec = MakeVec3(0.0, 0.0, 0.0);
        double rxGainLin = 1.0;

        if (input.config) {
            const Point3 rxPos = !path.nodes.empty() ? path.nodes.back().point : Point3{};
            AntennaModel rxAnt = BuildRxAntennaModel(*input.config, rxPos, "rx-finalize");
            // Rx天线独立极化矢量
            if (Length(rxAnt.polarization_vector) > 0.5) {
                rxPolVec = rxAnt.polarization_vector;
                rxPolImVec = MakeVec3(0.0, 0.0, 0.0);
            }
            // Per-angle Jones response (polarization + gain)
            if (rxAnt.pattern.loaded) {
                Vec3 incDir = Normalize(Subtract(path.nodes[path.nodes.size()-1].point,
                                                  path.nodes[path.nodes.size()-2].point));
                double thetaRad, phiRad;
                WorldToAntennaSpherical(incDir, rxAnt.forward, rxAnt.right, rxAnt.up,
                                        thetaRad, phiRad);
                double gainDBi = rxAnt.pattern.QueryGainDBi(thetaRad * 180.0 / kPi,
                                                             phiRad * 180.0 / kPi);
                rxGainLin = std::pow(10.0, gainDBi / 10.0);
                // Per-angle polarization if available
                if (rxAnt.pattern.polarization_loaded) {
                    double ptR, ptI, ppR, ppI;
                    rxAnt.pattern.QueryPolarization(thetaRad * 180.0 / kPi, phiRad * 180.0 / kPi,
                                                     ptR, ptI, ppR, ppI);
                    Vec3 thetaHat, phiHat;
                    AntennaSphericalBasisToWorld(rxAnt.forward, rxAnt.right, rxAnt.up,
                                                  thetaRad, phiRad, thetaHat, phiHat);
                    rxPolVec = MakeVec3(ptR*thetaHat.x + ppR*phiHat.x,
                                        ptR*thetaHat.y + ppR*phiHat.y,
                                        ptR*thetaHat.z + ppR*phiHat.z);
                    rxPolImVec = MakeVec3(ptI*thetaHat.x + ppI*phiHat.x,
                                          ptI*thetaHat.y + ppI*phiHat.y,
                                          ptI*thetaHat.z + ppI*phiHat.z);
                }
            }
        }

        // Stage D: 共轭匹配 — vr = E_inc · conj(h_rx)
        ComplexVec3 h_rx(Complex(rxPolVec.x, rxPolImVec.x),
                         Complex(rxPolVec.y, rxPolImVec.y),
                         Complex(rxPolVec.z, rxPolImVec.z));
        ComplexVec3 h_rx_conj(h_rx.x.Conj(), h_rx.y.Conj(), h_rx.z.Conj());
        Complex vr = ComplexDot(E_inc, h_rx_conj);
        // Stage D: 应用Rx增益
        vr = vr * std::sqrt(rxGainLin);

        result.amplitude_real = vr.re;
        result.amplitude_imag = vr.im;
        result.power_linear = vr.NormSq();
        result.polarization_vector = field.polarization_vector;
        result.polarization_imag = field.polarization_imag;
        result.polarization_magnitude = std::sqrt(Dot(field.polarization_vector, field.polarization_vector)
                                                + Dot(field.polarization_imag, field.polarization_imag));

        // 元数据 (复用下面旧路径的元数据代码)
        result.tx_antenna_id = field.tx_antenna_id;
        result.tx_antenna_source_type = field.tx_antenna_source_type;
        result.rx_antenna_id = field.rx_antenna_id;
        result.rx_antenna_source_type = field.rx_antenna_source_type;
        result.is_los = path.is_los;
        result.source_path_signature = path.path_signature;
        result.source_tag = "search_engine_real_output_vector";
        result.contains_transmission = path.contains_transmission;
        result.transmission_semantic_consumed = field.transmission_semantic_consumed;
        result.first_transmission_medium_in_id = field.last_transmission_medium_in_id;
        result.first_transmission_medium_out_id = field.last_transmission_medium_out_id;
        result.last_transmission_medium_in_id = field.last_transmission_medium_in_id;
        result.last_transmission_medium_out_id = field.last_transmission_medium_out_id;

        // AoA/AoD (复用下面旧路径逻辑)
        if (path.nodes.size() >= 2) {
            Vec3 aodDir = Normalize(Subtract(path.nodes[1].point, path.nodes[0].point));
            result.aod_theta_deg = std::acos(Clamp(aodDir.y, -1.0, 1.0)) * 180.0 / kPi;
            result.aod_phi_deg   = std::atan2(aodDir.z, aodDir.x) * 180.0 / kPi;
            std::size_t nIdx = path.nodes.size() - 1;
            Vec3 aoaDir = Normalize(Subtract(path.nodes[nIdx].point, path.nodes[nIdx-1].point));
            result.aoa_theta_deg = std::acos(Clamp(aoaDir.y, -1.0, 1.0)) * 180.0 / kPi;
            result.aoa_phi_deg   = std::atan2(aoaDir.z, aoaDir.x) * 180.0 / kPi;
        }
        return result;
    }

    // ── 旧标量路径 (兼容) ──
    // v8: Rx天线增益 (天线姿态 + 局部球坐标查询)
    double rxGainLin = 1.0;
    if (input.config) {
        const Point3 rxPos = !path.nodes.empty() ? path.nodes.back().point : Point3{};
        AntennaModel rxAnt = BuildRxAntennaModel(*input.config, rxPos, "rx-finalize");
        if (rxAnt.pattern.loaded && path.nodes.size() >= 2) {
            Vec3 incDir = Normalize(Subtract(path.nodes[path.nodes.size()-1].point,
                                              path.nodes[path.nodes.size()-2].point));
            double thetaRad, phiRad;
            WorldToAntennaSpherical(incDir, rxAnt.forward, rxAnt.right, rxAnt.up,
                                    thetaRad, phiRad);
            double thetaD = thetaRad * 180.0 / kPi;
            double phiD   = phiRad   * 180.0 / kPi;
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

    // v7 H10: AoD/AoA 从路径几何节点提取
    if (path.nodes.size() >= 2) {
        // AoD: 发射方向 Tx(node[0]) → 第一个交互点或Rx(node[1])
        Vec3 aodDir = Normalize(Subtract(path.nodes[1].point, path.nodes[0].point));
        result.aod_theta_deg = std::acos(Clamp(aodDir.y, -1.0, 1.0)) * 180.0 / kPi;  // Y-up zenith
        result.aod_phi_deg   = std::atan2(aodDir.z, aodDir.x) * 180.0 / kPi;

        // AoA: 到达方向 倒数第二个节点 → Rx(node[last])
        std::size_t nIdx = path.nodes.size() - 1;
        Vec3 aoaDir = Normalize(Subtract(path.nodes[nIdx].point, path.nodes[nIdx-1].point));
        result.aoa_theta_deg = std::acos(Clamp(aoaDir.y, -1.0, 1.0)) * 180.0 / kPi;
        result.aoa_phi_deg   = std::atan2(aoaDir.z, aoaDir.x) * 180.0 / kPi;
    }

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
