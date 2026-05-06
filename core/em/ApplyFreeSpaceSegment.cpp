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

    // Accumulate geometric length and time delay: delay = distance / c0
    field.total_length_m += segmentLengthM;
    field.delay_s += segmentLengthM / kC0;

    // Phase accumulation from propagation: delta_phi = -k * d = -2*pi * d / lambda
    // (negative sign by convention: e^{-j k d})
    field.phase_rad -= 6.28318530717958647693 * segmentLengthM / field.wavelength_m;

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
