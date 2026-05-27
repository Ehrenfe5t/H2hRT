// Declares the APS builder: converts per-path EM results into angle-vs-power
// entries using a placeholder angle metric (placeholder for future AoA/AoD).

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build an Angular Power Spectrum (APS) from per-path EM results.
/// Uses a placeholder angle metric (polarization_vector.x) as a stub
/// for future direction-of-arrival computation.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>APSResult with one entry per valid path.</returns>
APSResult BuildAPS(const EMPathResultSet& pathResults);

} // namespace rt
