// Builds the Angular Power Spectrum (APS) from per-path EM results: extracts a
// placeholder angle metric (currently polarization_vector.x) and associated power
// for each valid path. Intended as a stub for future AoA/AoD angular analysis.

#include "BuildAPS.h"

#include <cmath>

namespace rt {

/// <summary>
/// Build an Angular Power Spectrum (APS) from per-path EM results.
/// Each valid path maps to an APS entry with an angle metric and its power.
/// The current angle metric is a placeholder (uses polarization_vector.x)
/// pending integration of AoA/AoD direction-of-arrival computation.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>APSResult with one entry per valid path.</returns>
APSResult BuildAPS(const EMPathResultSet& pathResults)
{
    APSResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue;
        }
        APSEntry entry;
        // Placeholder angle metric: currently uses the x-component of the
        // polarization vector as a proxy for direction-of-arrival.
        // TODO: replace with proper AoA/AoD computation from path geometry.
        entry.angle_metric = item.polarization_vector.x;
        entry.power_linear = item.power_linear; // received linear power at this angle
        result.entries.push_back(entry);
    }
    return result;
}

} // namespace rt
