// Computes aggregate channel statistics from per-path EM results: valid path
// count, total and strongest path power, mean delay, and transmission path count.

#include "BuildChannelStatistics.h"

#include <cmath>

namespace rt {

/// <summary>
/// Compute aggregate channel statistics from a set of per-path EM results.
/// Accumulates:
///  - valid_path_count: number of paths with valid EM results
///  - total_power_linear: sum of linear power across all valid paths
///  - strongest_path_power_linear: maximum per-path linear power
///  - mean_delay_s: arithmetic mean of propagation delays
///  - mean_abs_phase_rad: placeholder (currently not computed)
///  - transmission_path_count: number of paths with consumed transmission
/// These statistics provide a compact summary of the multipath channel
/// for link budget analysis and system-level evaluation.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>ChannelStatistics with aggregate metrics.</returns>
ChannelStatistics BuildChannelStatistics(const EMPathResultSet& pathResults)
{
    ChannelStatistics result;
    double delaySum = 0.0;

    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue; // skip invalid results
        }
        ++result.valid_path_count;

        // Accumulate total received power (linear, non-coherent sum)
        result.total_power_linear += item.power_linear;

        // Track the strongest (maximum-power) path
        if (item.power_linear > result.strongest_path_power_linear)
        {
            result.strongest_path_power_linear = item.power_linear;
        }

        delaySum += item.delay_s;

        // Count paths that contain at least one transmission interaction
        if (item.contains_transmission)
        {
            ++result.transmission_path_count;
        }
    }

    // Compute arithmetic mean delay (avoids division by zero)
    if (result.valid_path_count > 0)
    {
        result.mean_delay_s = delaySum / static_cast<double>(result.valid_path_count);
    }

    return result;
}

} // namespace rt
