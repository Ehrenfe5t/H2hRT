// Builds the Power Delay Profile (PDP) from per-path EM results: maps each valid
// path into a delay-vs-power tap. Simpler than CIR -- no phase/amplitude, only power.

#include "BuildPDP.h"

namespace rt {

/// <summary>
/// Build a Power Delay Profile (PDP) from per-path EM results.
/// Each valid path contributes a PDP tap with its delay and linear power.
/// Unlike CIR, PDP discards phase and complex amplitude, retaining only
/// power-vs-delay for large-scale delay spread analysis.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>PDPResult with one tap per valid path.</returns>
PDPResult BuildPDP(const EMPathResultSet& pathResults)
{
    PDPResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue; // silently skip invalid paths
        }
        PDPTap tap;
        tap.delay_s = item.delay_s;       // propagation delay in seconds
        tap.power_linear = item.power_linear; // received linear power
        result.taps.push_back(tap);
    }
    return result;
}

} // namespace rt
