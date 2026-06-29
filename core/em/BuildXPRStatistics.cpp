// Builds the Cross-Polarization Ratio (XPR) statistics from per-path EM results.
// v11.3: mean, median, P10/P90 percentiles, power-weighted mean, min, max, CDF array.

#include "BuildXPRStatistics.h"

#include <algorithm>
#include <cmath>

namespace rt {

XPRStatistics BuildXPRStatistics(const EMPathResultSet& pathResults)
{
    XPRStatistics result;

    // ── Collect valid XPR values ──
    std::vector<double> xprValues;
    std::vector<double> pathPowers;
    double totalCoPower = 0.0;
    double totalCrossPower = 0.0;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        totalCoPower += item.co_pol_power_linear;
        totalCrossPower += item.cross_pol_power_linear;
        if (item.co_pol_power_linear <= 1e-30 && item.cross_pol_power_linear <= 1e-30) {
            ++result.zero_polarized_power_count;
            continue;
        }
        double xpr = item.xpr_dB;
        if (xpr <= -290.0) {
            xpr = -result.censor_limit_dB;
            ++result.pure_cross_path_count;
        } else if (xpr >= 290.0) {
            xpr = result.censor_limit_dB;
            ++result.pure_co_path_count;
        }
        xprValues.push_back(xpr);
        pathPowers.push_back(item.power_linear);
    }

    int n = static_cast<int>(xprValues.size());
    if (n == 0) return result;

    result.valid_path_count = n;

    // ── Sort for percentiles ──
    std::vector<double> sorted = xprValues;
    std::sort(sorted.begin(), sorted.end());
    result.xpr_values_dB = sorted;

    // ── Min / Max ──
    result.min_dB = sorted.front();
    result.max_dB = sorted.back();

    // ── Mean ──
    double sum = 0.0;
    for (double v : xprValues) sum += v;
    result.mean_dB = sum / static_cast<double>(n);

    // ── Median ──
    if (n % 2 == 1)
        result.median_dB = sorted[n / 2];
    else
        result.median_dB = 0.5 * (sorted[n / 2 - 1] + sorted[n / 2]);

    // ── P10 / P90 ──
    auto percentile = [&](double probability) {
        const double position = probability * static_cast<double>(n - 1);
        const int lower = static_cast<int>(std::floor(position));
        const int upper = static_cast<int>(std::ceil(position));
        const double fraction = position - lower;
        return sorted[lower] + fraction * (sorted[upper] - sorted[lower]);
    };
    result.p10_dB = percentile(0.10);
    result.p90_dB = percentile(0.90);

    // ── Power-weighted mean ──
    double powerSum = 0.0, weightedSum = 0.0;
    for (int i = 0; i < n; ++i) {
        weightedSum += xprValues[i] * pathPowers[i];
        powerSum += pathPowers[i];
    }
    if (powerSum > 0.0)
        result.power_weighted_mean_dB = weightedSum / powerSum;

    if (totalCoPower > 1e-30 && totalCrossPower > 1e-30)
        result.aggregate_xpr_dB = 10.0 * std::log10(totalCoPower / totalCrossPower);
    else if (totalCoPower > 1e-30)
        result.aggregate_xpr_dB = result.censor_limit_dB;
    else if (totalCrossPower > 1e-30)
        result.aggregate_xpr_dB = -result.censor_limit_dB;

    return result;
}

} // namespace rt
