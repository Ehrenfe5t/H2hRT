// v9 主线C: 宽带信道频域响应构建器
// Mode "fixed_gain": H(f) = Σ α_l(f0) · exp(-j2πf·τ_l)
// Mode "frequency_sweep_em": 逐频点重评估材料+Fresnel+相位

#pragma once

#include "EMProfile.h"
#include "../common/config/AppConfig.h"

#include <vector>

namespace rt {
class MaterialDatabase;
struct Scene;
}

namespace rt {

BroadbandChannelResult BuildBroadbandCFR_FixedGain(
    const EMPathResultSet& pathResults,
    const FrequencySweepConfig& sweepCfg,
    const ChannelObservationConfig& obsCfg);

BroadbandChannelResult BuildBroadbandCFR_FrequencySweep(
    const EMPathResultSet& pathResultsAtCenter,
    const FrequencySweepConfig& sweepCfg,
    const ChannelObservationConfig& obsCfg,
    const MaterialDatabase* matDb,
    const Scene* scene);

/// v9 C-8: 信道统计指标
struct ChannelStatisticsResult {
    double rms_delay_spread_s = 0.0;
    double mean_excess_delay_s = 0.0;
    double k_factor_dB = 0.0;
    double total_power_linear = 0.0;
    int path_count = 0;
    bool valid = false;
};
ChannelStatisticsResult ComputeChannelStatistics(const EMPathResultSet& pathResults);

} // namespace rt
