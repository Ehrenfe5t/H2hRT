// Initializes the FieldAccumulator: resolves Tx/Rx positions, builds fallback antennas,
// sets wavelength = c0/f, initializes unit amplitude, and applies Tx antenna pattern gain.

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

/// <summary>
/// Initializes the EM field accumulator at the transmitter. Sets frequency, wavelength,
/// initial amplitude/phase/power to unity, records Tx/Rx antenna metadata, and applies
/// the Tx antenna pattern gain along the initial ray launch direction.
/// </summary>
/// <param name="input">EM solver configuration and path data.</param>
/// <param name="field">Field accumulator to initialize (mutated in-place).</param>
/// <returns>true if initialization succeeded; false if invalid input or frequency.</returns>
bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field)
{
    if (input.config == nullptr || input.path == nullptr) return false;

    // Build fallback antenna models when the caller does not supply them
    const AntennaModel fallbackTx = BuildTxAntennaModel(*input.config, ResolveTxPosition(*input.path), "tx-ideal-default");
    const AntennaModel fallbackRx = BuildRxAntennaModel(*input.config, ResolveRxPosition(*input.path), "rx-ideal-default");
    const AntennaModel& txAnt = (input.tx_antenna != nullptr) ? *input.tx_antenna : fallbackTx;
    const AntennaModel& rxAnt = (input.rx_antenna != nullptr) ? *input.rx_antenna : fallbackRx;

    // Carrier frequency and free-space wavelength: lambda = c0 / f
    field.frequency_hz = input.config->em_solver.frequency_hz;
    if (field.frequency_hz <= 0.0) return false;
    field.wavelength_m = kC0 / field.frequency_hz;

    // Reset accumulators: total path length, delay, and phase start at zero
    field.total_length_m = 0.0; field.delay_s = 0.0; field.phase_rad = 0.0;
    // Initial field amplitude: unit magnitude, zero phase = 1 + 0j
    field.amplitude_real = 1.0; field.amplitude_imag = 0.0; field.power_linear = 1.0;
    field.free_space_amplitude_scale = 1.0; field.free_space_power_scale = 1.0;
    field.last_segment_length_m = 0.0;

    // Cache antenna identifiers for downstream metadata propagation
    field.tx_antenna_id = txAnt.antenna_id;
    field.tx_antenna_source_type = txAnt.source_type;
    field.rx_antenna_id = rxAnt.antenna_id;
    field.rx_antenna_source_type = rxAnt.source_type;

    // Start in free space (medium_id = 0), no transmission interaction consumed yet
    field.current_medium_id = 0;
    field.last_transmission_medium_in_id = -1;
    field.last_transmission_medium_out_id = -1;
    field.transmission_semantic_consumed = false;

    // 初始极化: Tx天线定义的方向为实部, 虚部初始化为零 (线极化出发)
    field.polarization_vector = txAnt.polarization_vector;
    field.polarization_imag = MakeVec3(0.0, 0.0, 0.0);

    // Medium attenuation accumulators start at zero (no lossy medium yet)
    field.media_attenuation_np = 0.0;
    field.current_attenuation_np_per_m = 0.0;

    // Apply Tx antenna pattern gain for the initial ray departure direction
    if (txAnt.pattern.loaded && !input.path->nodes.empty())
    {
        // Compute initial ray direction vector from node[0] (Tx) to node[1]
        Vec3 initDir = Normalize(Subtract(input.path->nodes[1].point, input.path->nodes[0].point));
        double thetaDeg, phiDeg;
        // Convert Cartesian direction to spherical coordinates (radians)
        CartesianToSpherical(initDir, thetaDeg, phiDeg);
        // Convert radians to degrees for antenna pattern lookup
        thetaDeg *= 180.0 / kPi; phiDeg *= 180.0 / kPi;
        double gainDBi = txAnt.pattern.QueryGainDBi(thetaDeg, phiDeg);
        // Convert dBi to linear gain: G_lin = 10^(dB/10)
        double gainLin = std::pow(10.0, gainDBi / 10.0);
        // Amplitude scales with sqrt(gain) because power = |amplitude|^2 scales linearly with gain
        field.amplitude_real *= std::sqrt(gainLin);
        field.amplitude_imag *= std::sqrt(gainLin);
        field.power_linear *= gainLin;
    }

    field.valid = true;
    return true;
}

} // namespace rt
