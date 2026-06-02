// Defines the dual-mode evaluation profile (PreciseEM / CoverageEM) and all
// aggregate output structures: CIR, PDP, APS, channel statistics, coverage, ISAC.

#pragma once

#include "EMPathResult.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// EM solver evaluation mode: PreciseEM preserves full complex field and polarization;
/// CoverageEM uses non-coherent power sum for large-scale coverage simulations.
/// </summary>
enum class EMSolveMode {
    PreciseEM = 0,  ///< High-fidelity mode: full complex field, polarization, and phase retained per path.
    CoverageEM      ///< Coverage mode: non-coherent power sum, reduced memory footprint.
};

/// <summary>
/// Configuration profile controlling which outputs the EM solver computes and
/// how path results are aggregated (coherent vs. non-coherent, thresholds, etc.).
/// </summary>
struct EMSolveProfile {
    EMSolveMode mode = EMSolveMode::PreciseEM;             ///< Active evaluation mode.
    bool keep_full_complex_field = true;                    ///< If true, retain real+imag field amplitude per path.
    bool keep_full_polarization = true;                     ///< If true, retain full 3D polarization vector per path.
    bool keep_phase_output = true;                          ///< If true, retain phase_rad in the result.
    bool enable_receiver_polarization_projection = true;    ///< If true, project received field onto Rx antenna polarization.
    bool enable_coherent_sum = true;                        ///< If true, sum complex amplitudes coherently for CIR.
    bool enable_noncoherent_power_sum = false;              ///< If true, also sum powers non-coherently (used in CoverageEM).
    int max_paths_per_receiver = 64;                        ///< Maximum number of paths to process per Rx.
    double min_power_threshold_linear = 0.0;                ///< Minimum linear power to retain a path in results.
    double delay_bin_s = 0.0;                               ///< v9 step24: delay bin width. 0 = per-path mode (no binning).
};

/// <summary>
/// Single tap in a Channel Impulse Response: delay, complex amplitude, power,
/// and IDs of the geometric paths contributing to this tap.
/// </summary>
struct CIRTap {
    double delay_s = 0.0;                       ///< Propagation delay in seconds.
    double amplitude_real = 0.0;                ///< Real part of the complex tap amplitude.
    double amplitude_imag = 0.0;                ///< Imaginary part of the complex tap amplitude.
    double power_linear = 0.0;                  ///< Linear power = |amplitude|^2.
    std::vector<int> contributing_path_ids;     ///< Path IDs that map to this delay bin.
};

/// <summary>
/// Channel Impulse Response: a set of delay-domain taps with optional coherent
/// summation flag.
/// </summary>
struct CIRResult {
    std::vector<CIRTap> taps;   ///< Ordered set of CIR taps.
    bool coherent = true;       ///< Whether taps were formed by coherent summing.
};

/// <summary>
/// Single tap in a Power Delay Profile: delay and power only (no phase).
/// </summary>
struct PDPTap {
    double delay_s = 0.0;             ///< Propagation delay in seconds.
    double power_linear = 0.0;        ///< Linear received power at this delay.
    // v9 step24: 分bin模式下区分相干/非相干功率
    double coherent_power_linear = 0.0;   ///< |Σα|² (相干叠加, 体现干涉效果).
    double incoherent_power_linear = 0.0; ///< Σ|α|² (非相干叠加, 体现能量到达).
};

/// <summary>
/// Power Delay Profile: delay-domain power distribution.
/// </summary>
struct PDPResult {
    std::vector<PDPTap> taps;   ///< Ordered set of PDP taps.
};

/// <summary>
/// Single entry in an Angular Power Spectrum.
/// </summary>
struct APSEntry {
    double angle_metric = 0.0;  ///< Placeholder angle metric (currently uses polarization_vector.x).
    double power_linear = 0.0;  ///< Linear power arriving at this angle.
};

/// <summary>
/// Angular Power Spectrum: angle-domain power distribution.
/// </summary>
struct APSResult {
    std::vector<APSEntry> entries;  ///< Ordered set of angle entries.
};

/// <summary>
/// Aggregate channel statistics: path count, total/strongest power, mean delay,
/// mean phase, transmission path count.
/// </summary>
struct ChannelStatistics {
    int valid_path_count = 0;               ///< Number of valid paths contributing to the statistics.
    double total_power_linear = 0.0;         ///< Sum of linear power across all valid paths.
    double strongest_path_power_linear = 0.0;///< Maximum per-path linear power.
    double mean_delay_s = 0.0;               ///< Arithmetic mean of path delays.
    double mean_abs_phase_rad = 0.0;         ///< Arithmetic mean of absolute phase (placeholder, currently unset).
    int transmission_path_count = 0;         ///< Count of paths that contain at least one transmission.
};

/// <summary>
/// Coverage result for one receiver location: total power, path count, and
/// average free-space loss.
/// </summary>
struct CoverageResult {
    double total_received_power_linear = 0.0; ///< Sum of linear power from all contributing paths.
    int contributing_path_count = 0;           ///< Number of paths above the power threshold.
    double average_free_space_loss_db = 0.0;   ///< Arithmetic mean of FSPL in dB across contributing paths.
};

/// <summary>
/// Integrated Sensing and Communication (ISAC) basic feature set extracted
/// from the multipath channel.
/// </summary>
struct ISACFeatureSet {
    int path_count = 0;                         ///< Total number of valid paths.
    double earliest_delay_s = 0.0;              ///< Minimum propagation delay (first-arriving path).
    double strongest_path_power_linear = 0.0;   ///< Maximum per-path linear power.
    double average_polarization_magnitude = 0.0;///< Arithmetic mean of polarization magnitude across paths.
    int transmission_path_count = 0;            ///< Count of paths that contain at least one transmission.
};

/// <summary>
/// Top-level aggregate result from the EM solver pipeline, bundling all
/// derived products (CIR, PDP, APS, statistics, coverage, ISAC features).
/// </summary>
struct EMAggregateResult {
    EMSolveProfile profile;          ///< The profile that produced this result.
    EMPathResultSet path_results;    ///< Raw per-path electromagnetic results.
    CIRResult cir;                   ///< Channel Impulse Response.
    PDPResult pdp;                   ///< Power Delay Profile.
    APSResult aps;                   ///< Angular Power Spectrum.
    ChannelStatistics statistics;    ///< Aggregate channel statistics.
    CoverageResult coverage;         ///< Coverage summary (used in CoverageEM mode).
    ISACFeatureSet isac_features;    ///< ISAC sensing features.
};

// ── v9 主线C: 宽带信道结果 ──

/// <summary>
/// Single frequency point in a CFR (Channel Frequency Response).
/// </summary>
struct CFRSample {
    double frequency_hz = 0.0;       ///< Frequency in Hz.
    double H_real = 0.0;             ///< Real part of H(f).
    double H_imag = 0.0;             ///< Imaginary part of H(f).
    double magnitude = 0.0;          ///< |H(f)|.
    double phase_rad = 0.0;          ///< arg(H(f)).
    double power_dB = 0.0;           ///< 10*log10(|H(f)|²).
};

/// <summary>
/// Broadband channel result: CFR H(f) over frequency, derived CIR, and metadata.
/// </summary>
struct BroadbandChannelResult {
    std::vector<CFRSample> cfr;              ///< Channel Frequency Response samples.
    std::vector<double> frequencies_hz;      ///< Frequency grid.
    CIRResult ideal_delta_cir;               ///< Ideal δ CIR from RT paths.
    CIRResult observed_cir;                  ///< Measurement-equivalent CIR (IFFT).
    PDPResult observed_pdp;                  ///< Measurement-equivalent PDP.
    double center_frequency_hz = 0.0;        ///< Center frequency.
    double bandwidth_hz = 0.0;               ///< Bandwidth.
    double delay_resolution_s = 0.0;         ///< 1/bandwidth — 可分辨时延.
    double max_unambiguous_delay_s = 0.0;    ///< N/bandwidth — 最大无模糊时延.
    std::string window_type;                  ///< Window function used.
    bool valid = false;
};

} // namespace rt
