// Builds the ISAC (Integrated Sensing and Communication) feature set from per-path
// EM results: path count, earliest delay, strongest path power, average polarization
// magnitude, and transmission path count.

#include "BuildISACFeatureSet.h"

namespace rt {

/// <summary>
/// Build ISAC basic features from per-path EM results.
/// Extracts sensing-relevant metrics:
///  - path_count: total number of valid propagation paths
///  - earliest_delay_s: minimum propagation delay (first-arriving path)
///  - strongest_path_power_linear: maximum per-path linear power
///  - average_polarization_magnitude: mean polarization magnitude across all paths
///  - transmission_path_count: number of paths with through-material transmission
/// These features support joint communication and radar sensing applications
/// where multipath channel characteristics inform target detection.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>ISACFeatureSet with sensing-relevant aggregate metrics.</returns>
ISACFeatureSet BuildISACFeatureSet(const EMPathResultSet& pathResults)
{
    ISACFeatureSet result;
    bool first = true;
    double polarizationSum = 0.0;

    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue; // skip invalid results
        }

        ++result.path_count;

        // Accumulate polarization magnitude for averaging
        polarizationSum += item.polarization_magnitude;

        // Count paths that experienced a transmission interaction
        if (item.transmission_semantic_consumed)
        {
            ++result.transmission_path_count;
        }

        // Track earliest (minimum) delay: the first-arriving path
        if (first || item.delay_s < result.earliest_delay_s)
        {
            result.earliest_delay_s = item.delay_s;
        }

        // Track the strongest (maximum-power) path
        if (item.power_linear > result.strongest_path_power_linear)
        {
            result.strongest_path_power_linear = item.power_linear;
        }

        first = false;
    }

    // Compute average polarization magnitude (avoids division by zero)
    if (result.path_count > 0)
    {
        result.average_polarization_magnitude = polarizationSum
            / static_cast<double>(result.path_count);
    }

    return result;
}

} // namespace rt
