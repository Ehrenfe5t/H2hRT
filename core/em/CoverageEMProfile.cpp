// Constructs the CoverageEM evaluation profile: non-coherent power sum, reduced
// path budget, power thresholding, and simplified output for large-scale coverage maps.

#include "CoverageEMProfile.h"

namespace rt {

/// <summary>
/// Build the CoverageEM evaluation profile.
/// CoverageEM mode uses non-coherent power summation (power = sum of |A_i|^2),
/// discards phase and full polarization to reduce memory, applies a power
/// threshold to filter weak paths, and limits to 16 paths per receiver.
/// Designed for large-scale coverage simulations where aggregate power is
/// the primary metric.
/// </summary>
/// <returns>EMSolveProfile configured for CoverageEM mode.</returns>
EMSolveProfile BuildCoverageEMProfile()
{
    EMSolveProfile profile;
    profile.mode = EMSolveMode::CoverageEM;
    profile.keep_full_complex_field = false;  // discard complex field; power-only
    profile.keep_full_polarization = false;   // discard polarization vector
    profile.keep_phase_output = false;        // discard phase information
    profile.enable_receiver_polarization_projection = false; // no polarization projection
    profile.enable_coherent_sum = false;      // no coherent summation for CIR
    profile.enable_noncoherent_power_sum = true; // sum powers non-coherently: P_total = sum(P_i)
    profile.max_paths_per_receiver = 16;      // smaller path budget for faster coverage
    // Minimum power threshold: filter paths below 1e-6 linear (1 uW relative)
    // to reduce noise in coverage maps.
    profile.min_power_threshold_linear = 1.0e-6;
    return profile;
}

} // namespace rt
