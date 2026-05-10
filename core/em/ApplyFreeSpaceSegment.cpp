// Per-segment free-space propagation: accumulates path length, delay, phase (k*d),
// and lossy-medium attenuation. FSPL is deferred to FinalizeAtReceiver.

#include "ApplyFreeSpaceSegment.h"
#include "../common/math/MathConstants.h"
#include <cmath>

namespace rt {

/// <summary>
/// Propagates the field over a free-space segment of given length. Accumulates total path
/// length and delay, advances the phase by -k*d, and applies medium-specific attenuation
/// (Np/m) if the ray is currently inside a lossy dielectric. FSPL is deferred to finalization.
/// </summary>
/// <param name="field">Current field accumulator (mutated with updated length/phase/attenuation).</param>
/// <param name="segmentLengthM">Propagation distance in meters (must be positive).</param>
/// <returns>true if propagation was applied; false if field invalid or parameters out of range.</returns>
bool ApplyFreeSpaceSegment(FieldAccumulator& field, double segmentLengthM)
{
    if (!field.valid || segmentLengthM <= 0.0 || field.wavelength_m <= 0.0)
        return false;

    // Accumulate geometric length. Time delay and phase use the medium's refractive index.
    field.total_length_m += segmentLengthM;
    double nEff = field.current_refractive_index;
    field.delay_s += segmentLengthM * nEff / kC0;       // v7 H2: τ = d·n/c₀
    field.phase_rad -= 6.28318530717958647693 * segmentLengthM * nEff / field.wavelength_m; // v7 H2: Δφ = -k₀·n·d

    // FSPL (Free-Space Path Loss) formula: L = (lambda / (4*pi*d))^2
    // is applied ONCE at FinalizeAtReceiver, NOT per-segment, to avoid compounding errors.
    // Here we only accumulate attenuation from lossy dielectric media (conductive/semiconductive).
    // Medium attenuation: A_media = exp(-alpha * d) where alpha is in Np/m.
    if (field.current_attenuation_np_per_m > 0.0)
        field.media_attenuation_np += field.current_attenuation_np_per_m * segmentLengthM;

    // Store last segment length for diffraction distance-parameter calculation
    field.last_segment_length_m = segmentLengthM;
    return true;
}

} // namespace rt
