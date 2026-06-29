// Initializes the FieldAccumulator: resolves Tx/Rx positions, builds fallback antennas,
// sets wavelength = c0/f, initializes unit amplitude, and applies Tx antenna pattern gain
// and polarization (v8: antenna orientation + per-angle polarization pattern).

#include "InitializeTxField.h"
#include "../antenna/AntennaFactory.h"
#include "../common/math/Vec3.h"
#include "../common/math/MathConstants.h"
#include "../common/math/CoordinateFrame.h"
#include <cmath>

namespace {

/// <summary>Extracts the transmitter position from the first node of the geometric path.</summary>
rt::Point3 ResolveTxPosition(const rt::GeometricPath& path) {
    return !path.nodes.empty() ? path.nodes.front().point : rt::Point3{};
}
/// <summary>Extracts the receiver position from the last node of the geometric path.</summary>
rt::Point3 ResolveRxPosition(const rt::GeometricPath& path) {
    return !path.nodes.empty() ? path.nodes.back().point : rt::Point3{};
}

} // namespace

namespace rt {

bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field)
{
    if (input.config == nullptr || input.path == nullptr) return false;

    AntennaModel fallbackTx;
    AntennaModel fallbackRx;
    const AntennaModel* txAntPtr = input.tx_antenna;
    const AntennaModel* rxAntPtr = input.rx_antenna;
    if (txAntPtr == nullptr) {
        fallbackTx = BuildTxAntennaModel(*input.config, ResolveTxPosition(*input.path), "tx-ideal-default");
        txAntPtr = &fallbackTx;
    }
    if (rxAntPtr == nullptr) {
        fallbackRx = BuildRxAntennaModel(*input.config, ResolveRxPosition(*input.path), "rx-ideal-default");
        rxAntPtr = &fallbackRx;
    }
    if (!txAntPtr->load_succeeded || !rxAntPtr->load_succeeded) return false;
    const AntennaModel& txAnt = *txAntPtr;
    const AntennaModel& rxAnt = *rxAntPtr;

    field.frequency_hz = input.config->em_solver.frequency_hz;
    if (field.frequency_hz <= 0.0) return false;
    field.wavelength_m = kC0 / field.frequency_hz;

    field.total_length_m = 0.0; field.delay_s = 0.0; field.phase_rad = 0.0;
    // The propagated vector is a complex power wave [sqrt(W)]. A specular
    // path is deterministic after refinement, so launch-ray support must not
    // scale Tx power.
    double txPowerW = std::pow(10.0, (input.tx_power_dBm - 30.0) / 10.0);
    field.tx_power_w = txPowerW;
    double txAmpScale = std::sqrt(std::max(0.0, txPowerW));
    field.amplitude_real = txAmpScale; field.amplitude_imag = 0.0; field.power_linear = txPowerW;
    field.free_space_amplitude_scale = 1.0; field.free_space_power_scale = 1.0;
    field.last_segment_length_m = 0.0;

    field.tx_antenna_id = txAnt.antenna_id;
    field.tx_antenna_source_type = txAnt.source_type;
    field.rx_antenna_id = rxAnt.antenna_id;
    field.rx_antenna_source_type = rxAnt.source_type;

    field.current_medium_id = 0;
    field.last_transmission_medium_in_id = -1;
    field.last_transmission_medium_out_id = -1;
    field.transmission_semantic_consumed = false;

    field.current_refractive_index = 1.0;
    field.media_attenuation_np = 0.0;
    field.current_attenuation_np_per_m = 0.0;

    // v8: Compute launch direction from path geometry
    Vec3 launchDir = MakeVec3(0.0, 1.0, 0.0); // fallback: straight up (Y-up convention)
    if (!input.path->nodes.empty() && input.path->nodes.size() >= 2) {
        launchDir = Normalize(Subtract(input.path->nodes[1].point, input.path->nodes[0].point));
    }

    // ── v8: Antenna orientation — convert world direction to antenna-local spherical ──
    double thetaDeg = 0.0, phiDeg = 0.0;
    if (txAnt.pattern.loaded || txAnt.pattern.polarization_loaded) {
        double thetaRad, phiRad;
        WorldToAntennaSpherical(launchDir, txAnt.forward, txAnt.right, txAnt.up,
                                thetaRad, phiRad);
        thetaDeg = thetaRad * 180.0 / kPi;
        phiDeg   = phiRad   * 180.0 / kPi;
    }

    // ── v8: Per-angle polarization pattern (Ludwig-3) ──
    if (txAnt.pattern.polarization_loaded) {
        double ptR, ptI, ppR, ppI;
        txAnt.pattern.QueryPolarization(thetaDeg, phiDeg, ptR, ptI, ppR, ppI);

        // Reconstruct world-space Jones vector from antenna-local spherical components
        double thetaRad, phiRad;
        WorldToAntennaSpherical(launchDir, txAnt.forward, txAnt.right, txAnt.up,
                                thetaRad, phiRad);
        Vec3 thetaHat, phiHat;
        AntennaLudwig3BasisToWorld(txAnt.forward, txAnt.right, txAnt.up,
                                   thetaRad, phiRad, thetaHat, phiHat);

        // Jones vector in world coords: E = polTheta*theta_hat + polPhi*phi_hat
        field.polarization_vector = MakeVec3(
            ptR*thetaHat.x + ppR*phiHat.x,
            ptR*thetaHat.y + ppR*phiHat.y,
            ptR*thetaHat.z + ppR*phiHat.z);
        field.polarization_imag = MakeVec3(
            ptI*thetaHat.x + ppI*phiHat.x,
            ptI*thetaHat.y + ppI*phiHat.y,
            ptI*thetaHat.z + ppI*phiHat.z);
        // Normalize Jones vector to unit magnitude
        double jLen = std::sqrt(Dot(field.polarization_vector, field.polarization_vector)
                              + Dot(field.polarization_imag, field.polarization_imag));
        if (jLen > 1e-12) {
            double inv = 1.0 / jLen;
            field.polarization_vector = Scale(field.polarization_vector, inv);
            field.polarization_imag    = Scale(field.polarization_imag,    inv);
        }
    } else {
        // Default: fixed linear polarization from antenna model
        field.polarization_vector = txAnt.polarization_vector;
        field.polarization_imag = MakeVec3(0.0, 0.0, 0.0);
    }

    // ── Tx antenna gain (with antenna-local angle query) ──
    if (txAnt.pattern.loaded) {
        double gainDBi = txAnt.pattern.QueryGainDBi(thetaDeg, phiDeg);
        double gainLin = std::pow(10.0, gainDBi / 10.0);
        field.amplitude_real *= std::sqrt(gainLin);
        field.amplitude_imag *= std::sqrt(gainLin);
        field.power_linear *= gainLin;
    }

    // ── v9 B1-d: 初始化复矢量电场 ──
    // 从已构建的极化矢量构造世界坐标复电场
    double polMag = std::sqrt(Dot(field.polarization_vector, field.polarization_vector)
                            + Dot(field.polarization_imag, field.polarization_imag));
    if (polMag > 1e-12) {
        double invMag = 1.0 / polMag;
        double amp = std::sqrt(std::max(0.0, field.power_linear));
        // E_world = amp * (P_real + j*P_imag) / |P|
        field.electric_field_world = ComplexVec3(
            Complex(field.polarization_vector.x * amp * invMag,
                    field.polarization_imag.x * amp * invMag),
            Complex(field.polarization_vector.y * amp * invMag,
                    field.polarization_imag.y * amp * invMag),
            Complex(field.polarization_vector.z * amp * invMag,
                    field.polarization_imag.z * amp * invMag));
    } else {
        // 默认: Y轴垂直极化, 单位幅度
        field.electric_field_world = ComplexVec3(
            Complex(0.0, 0.0),
            Complex(std::sqrt(field.power_linear), 0.0),
            Complex(0.0, 0.0));
    }
    field.vector_field_valid = true;
    field.SyncLegacyFields();

    field.valid = true;
    return true;
}

} // namespace rt
