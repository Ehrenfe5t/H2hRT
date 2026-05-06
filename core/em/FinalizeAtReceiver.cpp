// Finalizes the EM field at the receiver: applies FSPL once (lambda/(4*pi*d)),
// medium attenuation (exp(-alpha_tot)), Rx antenna pattern gain, and packs the
// complete EMPathResult with amplitude, power, polarization, and metadata.

#include "FinalizeAtReceiver.h"
#include "../common/math/Vec3.h"
#include "../common/math/MathConstants.h"
#include "../common/math/CoordinateFrame.h"
#include "../antenna/AntennaFactory.h"
#include <cmath>

namespace rt {

/// <summary>
/// Produces the final EM path result at the receiver. Applies the free-space path loss (FSPL)
/// once over the total accumulated path length, multiplies by the cumulative medium attenuation,
/// applies the Rx antenna pattern gain along the incident direction, and packs amplitude,
/// phase, power, polarization, and metadata into an EMPathResult struct.
/// </summary>
/// <param name="field">Field accumulator carrying all accumulated propagation/reflection/transmission/diffraction state.</param>
/// <param name="path">Geometric path whose total length and last segment direction are used for FSPL and Rx gain.</param>
/// <param name="input">Solver input providing config and scene for antenna model construction.</param>
/// <returns>Fully populated EMPathResult with final complex amplitude and metadata.</returns>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path, const EMSolverInput& input)
{
    EMPathResult result;
    result.path_id = path.path_id;
    result.valid = field.valid && path.valid;
    if (!result.valid) return result;

    // Copy accumulated propagation totals
    result.total_length_m = field.total_length_m;
    result.delay_s = field.delay_s;       // total delay = sum(d_i / c0)
    result.phase_rad = field.phase_rad;    // cumulative phase from propagation and interactions
    result.wavelength_m = field.wavelength_m;

    // --- FSPL (Free-Space Path Loss) applied ONCE here ---
    // FSPL amplitude factor: A_fspl = lambda / (4 * pi * d)
    // This is the field amplitude at distance d from an isotropic radiator.
    // Power ratio: P/P_tx = (lambda / (4*pi*d))^2
    double fsplAmp = field.wavelength_m / (6.28318530717958647693 * 2.0 * field.total_length_m);
    result.free_space_amplitude_scale = fsplAmp;
    result.free_space_power_scale = fsplAmp * fsplAmp;
    // FSPL in dB: L_dB = -10*log10(P/P_tx) = -10*log10(power_scale)
    result.free_space_loss_db = (result.free_space_power_scale > 0.0)
        ? (-10.0 * std::log10(result.free_space_power_scale)) : 0.0;

    // --- Medium attenuation ---
    // Total accumulated attenuation in nepers: alpha_tot = sum(alpha_i * d_i)
    // Amplitude loss factor: exp(-alpha_tot) converts nepers to linear amplitude loss.
    double mediaLoss = std::exp(-field.media_attenuation_np);
    // Combined scaling factor: FSPL amplitude * medium attenuation
    double totalScale = fsplAmp * mediaLoss;

    // --- Rx antenna gain from pattern ---
    double rxGainLin = 1.0;
    const Point3 rxPos = !path.nodes.empty() ? path.nodes.back().point : Point3{};
    AntennaModel rxAnt = BuildRxAntennaModel(*input.config, rxPos, "rx-finalize");
    if (rxAnt.pattern.loaded && path.nodes.size() >= 2)
    {
        // Incident direction at Rx: from second-last node toward last node
        Vec3 incDir = Normalize(Subtract(path.nodes[path.nodes.size()-1].point,
                                         path.nodes[path.nodes.size()-2].point));
        double thetaD, phiD;
        // Convert Cartesian to spherical angles (radians)
        CartesianToSpherical(incDir, thetaD, phiD);
        // Convert radians to degrees for antenna pattern lookup
        thetaD *= 180.0 / kPi; phiD *= 180.0 / kPi;
        double gainDBi = rxAnt.pattern.QueryGainDBi(thetaD, phiD);
        // Convert dBi to linear gain: G_lin = 10^(dB/10)
        rxGainLin = std::pow(10.0, gainDBi / 10.0);
    }

    // Rx antenna gain scales amplitude by sqrt(gain): |A| -> |A| * sqrt(G)
    totalScale *= std::sqrt(rxGainLin);

    // Apply combined scaling to complex amplitude
    result.amplitude_real = field.amplitude_real * totalScale;
    result.amplitude_imag = field.amplitude_imag * totalScale;
    // Power = |amplitude|^2 = real^2 + imag^2
    result.power_linear = result.amplitude_real * result.amplitude_real
                        + result.amplitude_imag * result.amplitude_imag;

    // --- Polarization ---
    result.polarization_vector = field.polarization_vector;
    double polMagSq = Dot(field.polarization_vector, field.polarization_vector);
    result.polarization_magnitude = std::sqrt(polMagSq);

    // --- Metadata propagation ---
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

/// <summary>
/// Convenience overload of FinalizeAtReceiver without EMSolverInput. Constructs a dummy
/// solver input and delegates to the full overload. Provided for callers that do not need
/// Rx antenna pattern lookup.
/// </summary>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path) {
    EMSolverInput dummy; dummy.config = nullptr;
    return FinalizeAtReceiver(field, path, dummy);
}

} // namespace rt
