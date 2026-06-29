// Computes aggregate channel statistics from per-path EM results: valid path
// count, total and strongest path power, mean delay, transmission path count,
// and (v11.3) RMS delay spread, K-factor, effective path count, strongest path delay.

#include "BuildChannelStatistics.h"

#include <cmath>
#include <algorithm>

namespace rt {

/// <summary>
/// Compute aggregate channel statistics from a set of per-path EM results.
/// Accumulates:
///  - valid_path_count, total_power_linear, strongest_path_power_linear
///  - mean_delay_s, transmission_path_count (existing)
///  - rms_delay_spread_s: power-weighted RMS delay spread
///  - k_factor_dB: 10*log10(P_LOS / P_NLOS), set to +300 if pure LOS, -300 if pure NLOS
///  - effective_path_count: paths with power >= strongest/100 (-20 dB)
///  - strongest_path_delay_s: delay of the strongest path
/// </summary>
ChannelStatistics BuildChannelStatistics(const EMPathResultSet& pathResults)
{
    ChannelStatistics result;
    double delaySum = 0.0;
    double losPower = 0.0;
    double weightedDelaySum = 0.0;
    double powerSum = 0.0;

    // ── Pass 1: accumulate basic metrics ──
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        ++result.valid_path_count;

        result.total_power_linear += item.power_linear;

        if (item.power_linear > result.strongest_path_power_linear)
        {
            result.strongest_path_power_linear = item.power_linear;
            result.strongest_path_delay_s = item.delay_s;
        }

        delaySum += item.delay_s;
        weightedDelaySum += item.power_linear * item.delay_s;
        powerSum += item.power_linear;

        if (item.is_los)
            losPower += item.power_linear;

        if (item.contains_transmission)
            ++result.transmission_path_count;
    }

    if (result.valid_path_count == 0) return result;

    // ── Mean delay (arithmetic) ──
    result.mean_delay_s = delaySum / static_cast<double>(result.valid_path_count);

    // ── v11.3: RMS delay spread (power-weighted) ──
    if (powerSum > 0.0) {
        double meanWeightedDelay = weightedDelaySum / powerSum;
        result.power_weighted_mean_delay_s = meanWeightedDelay;
        double varSum = 0.0;
        for (const EMPathResult& item : pathResults.results) {
            if (!item.valid) continue;
            double d = item.delay_s - meanWeightedDelay;
            varSum += item.power_linear * d * d;
        }
        result.rms_delay_spread_s = std::sqrt(varSum / powerSum);
    }

    // ── v11.3: K-factor ──
    result.los_power_linear = losPower;
    result.nlos_power_linear = std::max(0.0, result.total_power_linear - losPower);
    double nlosPower = result.nlos_power_linear;
    if (losPower > 1e-30 && nlosPower > 1e-30) {
        result.k_factor_dB = 10.0 * std::log10(losPower / nlosPower);
    } else if (losPower > 1e-30) {
        result.k_factor_dB = 300.0;  // pure LOS
    } else {
        result.k_factor_dB = -300.0; // pure NLOS
    }

    // ── v11.3: Effective path count (within 20 dB of strongest) ──
    double threshold = result.strongest_path_power_linear / 100.0; // -20 dB
    for (const EMPathResult& item : pathResults.results) {
        if (item.valid && item.power_linear >= threshold)
            ++result.effective_path_count;
    }

    return result;
}

} // namespace rt
