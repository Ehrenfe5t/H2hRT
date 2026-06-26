// Builds the Power Delay Profile (PDP) from per-path EM results.
// v9 step24: 支持延迟分bin PWM + 相干/非相干功率分离.

#include "BuildPDP.h"
#include "../common/math/Complex.h"
#include <map>

namespace rt {

PDPResult BuildPDP(const EMPathResultSet& pathResults)
{
    PDPResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        PDPTap tap;
        tap.delay_s = item.delay_s;
        tap.power_linear = item.power_linear;
        result.taps.push_back(tap);
    }
    return result;
}

// v9 step24: binned PDP with coherent + incoherent power
PDPResult BuildPDP(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    PDPResult result;

    if (profile.delay_bin_s > 0.0) {
        struct BinData { Complex sum; double incoherentPwr; double centerDelay; };
        std::map<int, BinData> bins;

        for (const auto& item : pathResults.results) {
            if (!item.valid) continue;

            int binIdx = static_cast<int>(std::floor(item.delay_s / profile.delay_bin_s));
            auto& bin = bins[binIdx];
            bin.centerDelay = (binIdx + 0.5) * profile.delay_bin_s;

            Complex alpha(item.amplitude_real, item.amplitude_imag);
            bin.sum = bin.sum + alpha;
            bin.incoherentPwr += alpha.NormSq();
        }

        for (auto& [idx, bin] : bins) {
            PDPTap tap;
            tap.delay_s = bin.centerDelay;
            tap.coherent_power_linear = bin.sum.NormSq();   // |Σα|²
            tap.incoherent_power_linear = bin.incoherentPwr; // Σ|α|²
            tap.power_linear = tap.coherent_power_linear;    // 默认=相干
            result.taps.push_back(tap);
        }
        return result;
    }

    // fallback: per-path (backward compat)
    return BuildPDP(pathResults);
}

} // namespace rt
