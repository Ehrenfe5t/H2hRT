// Declares the coverage result builder: sums non-coherent received power from
// qualifying paths for grid-based coverage maps, with average FSPL.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build a CoverageResult from per-path EM results.
/// Sums non-coherent power from all paths above the threshold, counts
/// contributing paths, and computes average free-space loss in dB.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <param name="profile">Profile controlling the power threshold.</param>
/// <returns>CoverageResult with total power, path count, and average FSPL.</returns>
CoverageResult BuildCoverageResult(const EMPathResultSet& pathResults, const EMSolveProfile& profile);

} // namespace rt
