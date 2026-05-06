// Builds the CoverageResult from per-path EM results: sums received power from
// all qualifying paths, counts contributing paths, and averages free-space loss.

#include "BuildCoverageResult.h"

namespace rt {

/// <summary>
/// Build a CoverageResult from per-path EM results.
/// Accumulates total received power (non-coherent sum of all qualifying paths
/// above the power threshold), counts the number of contributing paths, and
/// computes the arithmetic mean of free-space path loss in dB across those paths.
/// Used in CoverageEM mode for grid-based coverage maps.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <param name="profile">Profile controlling the minimum power threshold.</param>
/// <returns>CoverageResult with total power, path count, and average FSPL.</returns>
CoverageResult BuildCoverageResult(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    CoverageResult result;
    double lossSum = 0.0;

    for (const EMPathResult& item : pathResults.results)
    {
        // Filter: skip invalid paths or those below the power threshold
        if (!item.valid || item.power_linear < profile.min_power_threshold_linear)
        {
            continue;
        }

        // Non-coherent power summation: P_total = sum(P_i) for each qualifying path
        result.total_received_power_linear += item.power_linear;
        ++result.contributing_path_count;

        // Accumulate FSPL in dB for averaging
        lossSum += item.free_space_loss_db;
    }

    // Compute average FSPL across contributing paths (avoids division by zero)
    if (result.contributing_path_count > 0)
    {
        result.average_free_space_loss_db = lossSum
            / static_cast<double>(result.contributing_path_count);
    }

    return result;
}

} // namespace rt
