// Constructs the PreciseEM evaluation profile: full complex field, polarization,
// phase, coherent summation, and higher path budget for high-fidelity analysis.

#include "PreciseEMProfile.h"

namespace rt {

/// <summary>
/// Build the PreciseEM evaluation profile.
/// PreciseEM mode retains full complex field amplitude (real+imag), 3D polarization
/// vectors, phase at each path, and uses coherent summation for CIR construction.
/// Designed for high-fidelity channel analysis with per-path detail.
/// </summary>
/// <returns>EMSolveProfile configured for PreciseEM mode.</returns>
EMSolveProfile BuildPreciseEMProfile()
{
    EMSolveProfile profile;
    profile.mode = EMSolveMode::PreciseEM;
    profile.keep_full_complex_field = true;   // retain real+imag per path
    profile.keep_full_polarization = true;    // retain full 3D polarization vector per path
    profile.keep_phase_output = true;         // retain phase_rad in result
    profile.enable_receiver_polarization_projection = true; // project onto Rx polarization
    profile.enable_coherent_sum = true;       // CIR: sum complex amplitudes coherently
    profile.enable_noncoherent_power_sum = false; // no non-coherent power sum in precise mode
    profile.max_paths_per_receiver = 64;      // up to 64 paths per Rx for detailed analysis
    profile.min_power_threshold_linear = 0.0; // no power threshold (keep all paths)
    return profile;
}

} // namespace rt
