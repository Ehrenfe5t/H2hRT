// Builds the Channel Impulse Response (CIR) from per-path EM results: maps each
// valid path into a delay-domain tap with complex amplitude, power, and contributing
// path IDs. Respects the profile's power threshold and coherent flag.

#include "BuildCIR.h"

namespace rt {

/// <summary>
/// Build a Channel Impulse Response (CIR) from a set of per-path EM results.
/// Each valid path above the power threshold becomes a CIR tap with its delay,
/// complex amplitude, and power. The contributing_path_ids list records which
/// geometric paths map to each tap (for diagnostic traceability).
/// The coherent flag is copied from the profile for downstream use.
/// </summary>
/// <param name="pathResults">Collection of per-path EM results from the solver.</param>
/// <param name="profile">Evaluation profile dictating power threshold and coherent sum flag.</param>
/// <returns>CIRResult with one tap per valid path above the threshold.</returns>
CIRResult BuildCIR(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    CIRResult result;
    result.coherent = profile.enable_coherent_sum; // whether taps represent coherent sums

    for (const EMPathResult& item : pathResults.results)
    {
        // Filter: skip invalid paths or those below power threshold
        if (!item.valid || item.power_linear < profile.min_power_threshold_linear)
        {
            continue;
        }

        // Create a CIR tap: delay, complex amplitude, power
        CIRTap tap;
        tap.delay_s = item.delay_s;
        tap.amplitude_real = item.amplitude_real;
        tap.amplitude_imag = item.amplitude_imag;
        tap.power_linear = item.power_linear;
        // Record contributing path ID for traceability
        tap.contributing_path_ids.push_back(item.path_id);
        result.taps.push_back(tap);
    }
    return result;
}

} // namespace rt
