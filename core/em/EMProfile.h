// 文件目标：
// - 定义模块5批次8的双模式求值 profile 与输出层结果结构。
//
// 主要功能：
// - 提供 PreciseEM / CoverageEM 两种模式的统一配置结构；
// - 定义 CIR / PDP / APS / ChannelStatistics / CoverageResult / ISACFeatureSet；
// - 为批次8多路径汇总与结果派生提供稳定类型基础。

#pragma once

#include "EMPathResult.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 模块5求值模式枚举。
/// </summary>
enum class EMSolveMode {
    PreciseEM = 0,
    CoverageEM
};

/// <summary>
/// 模块5求值 profile 结构。
/// </summary>
struct EMSolveProfile {
    EMSolveMode mode = EMSolveMode::PreciseEM;
    bool keep_full_complex_field = true;
    bool keep_full_polarization = true;
    bool keep_phase_output = true;
    bool enable_receiver_polarization_projection = true;
    bool enable_coherent_sum = true;
    bool enable_noncoherent_power_sum = false;
    int max_paths_per_receiver = 64;
    double min_power_threshold_linear = 0.0;
};

/// <summary>
/// CIR 抽头结构。
/// </summary>
struct CIRTap {
    double delay_s = 0.0;
    double amplitude_real = 0.0;
    double amplitude_imag = 0.0;
    double power_linear = 0.0;
    std::vector<int> contributing_path_ids;
};

/// <summary>
/// CIR 结果结构。
/// </summary>
struct CIRResult {
    std::vector<CIRTap> taps;
    bool coherent = true;
};

/// <summary>
/// PDP 抽头结构。
/// </summary>
struct PDPTap {
    double delay_s = 0.0;
    double power_linear = 0.0;
};

/// <summary>
/// PDP 结果结构。
/// </summary>
struct PDPResult {
    std::vector<PDPTap> taps;
};

/// <summary>
/// APS 条目结构。
/// </summary>
struct APSEntry {
    double angle_metric = 0.0;
    double power_linear = 0.0;
};

/// <summary>
/// APS 结果结构。
/// </summary>
struct APSResult {
    std::vector<APSEntry> entries;
};

/// <summary>
/// 信道统计结构。
/// </summary>
struct ChannelStatistics {
    int valid_path_count = 0;
    double total_power_linear = 0.0;
    double strongest_path_power_linear = 0.0;
    double mean_delay_s = 0.0;
    double mean_abs_phase_rad = 0.0;
    int transmission_path_count = 0;
};

/// <summary>
/// 覆盖结果结构。
/// </summary>
struct CoverageResult {
    double total_received_power_linear = 0.0;
    int contributing_path_count = 0;
    double average_free_space_loss_db = 0.0;
};

/// <summary>
/// 通感基础特征结构。
/// </summary>
struct ISACFeatureSet {
    int path_count = 0;
    double earliest_delay_s = 0.0;
    double strongest_path_power_linear = 0.0;
    double average_polarization_magnitude = 0.0;
    int transmission_path_count = 0;
};

/// <summary>
/// 模块5批次8汇总输出结构。
/// </summary>
struct EMAggregateResult {
    EMSolveProfile profile;
    EMPathResultSet path_results;
    CIRResult cir;
    PDPResult pdp;
    APSResult aps;
    ChannelStatistics statistics;
    CoverageResult coverage;
    ISACFeatureSet isac_features;
};

} // namespace rt
