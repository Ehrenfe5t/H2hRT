// Builds the Channel Impulse Response (CIR) from per-path EM results: maps each
// valid path into a delay-domain tap with complex amplitude, power, and contributing
// path IDs. Respects the profile's power threshold and coherent flag.

#include "BuildCIR.h"
#include "../common/math/Complex.h"
#include <map>

namespace rt {

/// <summary>
/// Build a Channel Impulse Response (CIR) from a set of per-path EM results.
/// Each valid path above the power threshold becomes a CIR tap with its delay,
/// complex amplitude, and power. The contributing_path_ids list records which
/// geometric paths map to each tap (for diagnostic traceability).
/// The coherent flag is copied from the profile for downstream use.
/// </summary>
/// <param name="pathResults">Collection of per-path EM results from the solver.</param>
/// <param name="profile">Evaluation profile dictating power threshold and coherent sum flag.</param>
/// <returns>CIRResult with one tap per valid path above the threshold.</returns>
CIRResult BuildCIR(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    CIRResult result;
    result.coherent = profile.enable_coherent_sum;

    // v9 step24: 延迟分bin相干CIR
    if (profile.delay_bin_s > 0.0 && profile.enable_coherent_sum) {
        // 找出所有有效路径, 按delay分bin
        struct BinData { Complex sum; double centerDelay; std::vector<int> ids; };
        std::map<int, BinData> bins; // bin index → data

        for (const auto& item : pathResults.results) {
            if (!item.valid || item.power_linear < profile.min_power_threshold_linear) continue;

            int binIdx = static_cast<int>(std::floor(item.delay_s / profile.delay_bin_s));
            auto& bin = bins[binIdx];
            bin.centerDelay = (binIdx + 0.5) * profile.delay_bin_s;
            // 复振幅 = amplitude * exp(j*phase)
            Complex alpha(item.amplitude_real * std::cos(item.phase_rad),
                          item.amplitude_real * std::sin(item.phase_rad));
            bin.sum = bin.sum + alpha;
            bin.ids.push_back(item.path_id);
        }

        for (auto& [idx, bin] : bins) {
            CIRTap tap;
            tap.delay_s = bin.centerDelay;
            tap.amplitude_real = bin.sum.re;
            tap.amplitude_imag = bin.sum.im;
            tap.power_linear = bin.sum.NormSq(); // 相干功率: |Σα|²
            tap.contributing_path_ids = std::move(bin.ids);
            result.taps.push_back(tap);
        }
        return result;
    }

    // 无分bin: 每路径一个tap (向后兼容)
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid || item.power_linear < profile.min_power_threshold_linear) continue;

        CIRTap tap;
        tap.delay_s = item.delay_s;
        tap.amplitude_real = item.amplitude_real * std::cos(item.phase_rad);
        tap.amplitude_imag = item.amplitude_real * std::sin(item.phase_rad);
        tap.power_linear = item.power_linear;
        tap.contributing_path_ids.push_back(item.path_id);
        result.taps.push_back(tap);
    }
    return result;
}

} // namespace rt
