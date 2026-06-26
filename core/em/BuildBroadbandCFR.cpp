// v9 主线C C-3: 固定路径增益CFR — 窄带近似宽带信道
// H(f) = Σ_l α_l(f0) · exp(-j2πf·τ_l)
// 适用: 带宽/中心频率 << 1 的场景 (如3GHz中心、200MHz带宽)

#include "BuildBroadbandCFR.h"
#include "../common/math/MathConstants.h"
#include "../common/math/Complex.h"
#include "../common/material/MaterialDatabase.h"
#include "../scene/Scene.h"

#include <cmath>
#include <algorithm>
#include <map>
#include <cstdio>

namespace rt {

BroadbandChannelResult BuildBroadbandCFR_FixedGain(
    const EMPathResultSet& pathResults,
    const FrequencySweepConfig& sweepCfg,
    const ChannelObservationConfig& obsCfg)
{
    BroadbandChannelResult result;
    if (!sweepCfg.enabled || sweepCfg.point_count < 1) {
        result.valid = false;
        return result;
    }

    result.center_frequency_hz = sweepCfg.center_hz;
    result.bandwidth_hz = sweepCfg.bandwidth_hz;
    result.window_type = obsCfg.window_type;
    result.delay_resolution_s = 1.0 / sweepCfg.bandwidth_hz;
    result.max_unambiguous_delay_s = sweepCfg.point_count / sweepCfg.bandwidth_hz;

    // ── Step 1: 收集所有有效路径的复幅度 α_l(f0) + 时延 τ_l ──
    struct PathAlpha {
        Complex alpha;    // 复幅度 at center frequency
        double delay_s;   // 传播时延
    };
    std::vector<PathAlpha> paths;
    for (const auto& item : pathResults.results) {
        if (!item.valid) continue;
        // v9 StageE: use Complex(amplitude_real, amplitude_imag) directly
        // amplitude_imag already encodes phase_rad via FinalizeAtReceiver
        Complex alpha(item.amplitude_real, item.amplitude_imag);
        paths.push_back({alpha, item.delay_s});
    }

    // ── Step 2: 生成频率网格 ──
    double fStart = sweepCfg.center_hz - sweepCfg.bandwidth_hz * 0.5;
    double fStop  = sweepCfg.center_hz + sweepCfg.bandwidth_hz * 0.5;
    int N = sweepCfg.point_count;

    result.frequencies_hz.resize(N);
    result.cfr.resize(N);

    for (int i = 0; i < N; ++i) {
        double f;
        if (sweepCfg.spacing == "log") {
            double ratio = (N > 1) ? static_cast<double>(i) / (N - 1) : 0.0;
            f = fStart * std::pow(fStop / fStart, ratio);
        } else {
            f = fStart + (fStop - fStart) * i / std::max(1, N - 1);
        }
        result.frequencies_hz[i] = f;

        // ── H(f) = Σ α_l · exp(-j·2π·f·τ_l) ──
        Complex H(0.0, 0.0);
        for (const auto& p : paths) {
            double phase = -kTwoPi * f * p.delay_s;  // -jωτ
            Complex rot(std::cos(phase), std::sin(phase));
            H = H + p.alpha * rot;
        }

        result.cfr[i].frequency_hz = f;
        result.cfr[i].H_real = H.re;
        result.cfr[i].H_imag = H.im;
        result.cfr[i].magnitude = H.Norm();
        result.cfr[i].phase_rad = H.Arg();
        result.cfr[i].power_dB = (H.NormSq() > 0.0) ? (10.0 * std::log10(H.NormSq())) : -300.0;
    }

    // ── Step 3: IFFT → 可观测CIR (h(tau) = IFFT(H(f)·W(f))) ──
    if (obsCfg.export_observed_cir_ifft && N >= 2) {
        // 窗函数权重
        std::vector<double> window(N, 1.0);
        if (obsCfg.window_type == "hann") {
            for (int i = 0; i < N; ++i)
                window[i] = 0.5 * (1.0 - std::cos(kTwoPi * i / (N - 1)));
        } else if (obsCfg.window_type == "hamming") {
            for (int i = 0; i < N; ++i)
                window[i] = 0.54 - 0.46 * std::cos(kTwoPi * i / (N - 1));
        }
        // rect: window[i] = 1.0 (default)

        // IFFT: h[k] = (1/N) Σ H(f_i) · W(f_i) · exp(+j·2π·i·k/N)
        // 时延轴: tau[k] = k · Δτ, where Δτ = 1/B (delay resolution)
        int nBins = std::min(N, 2048); // max 2048 delay bins

        // v10.2 B2修复: 移除死代码块 (lines 100-105, 原计算expTerm但未累积到sum)

        std::vector<Complex> H_windowed(N);
        for (int i = 0; i < N; ++i) {
            H_windowed[i] = Complex(result.cfr[i].H_real * window[i],
                                     result.cfr[i].H_imag * window[i]);
        }

        std::vector<Complex> h(nBins, Complex(0.0, 0.0));
        for (int k = 0; k < nBins; ++k) {
            Complex sum(0.0, 0.0);
            for (int i = 0; i < N; ++i) {
                double phase = kTwoPi * i * k / N;
                Complex rot(std::cos(phase), std::sin(phase));
                sum = sum + H_windowed[i] * rot;
            }
            h[k] = Complex(sum.re / N, sum.im / N);

            double tau_k = k * result.delay_resolution_s;
            CIRTap tap;
            tap.delay_s = tau_k;
            tap.amplitude_real = h[k].re;
            tap.amplitude_imag = h[k].im;
            tap.power_linear = h[k].NormSq();
            result.observed_cir.taps.push_back(tap);
        }
        result.observed_cir.coherent = true;

        // Observed PDP from CIR taps
        for (auto& tap : result.observed_cir.taps) {
            PDPTap ptap;
            ptap.delay_s = tap.delay_s;
            ptap.power_linear = tap.power_linear;
            ptap.coherent_power_linear = tap.power_linear;
            ptap.incoherent_power_linear = tap.power_linear;
            result.observed_pdp.taps.push_back(ptap);
        }
    }

    // ── Step 4: 理想δ CIR (路径直接分bin) ──
    if (obsCfg.export_ideal_delta_cir) {
        double binS = (obsCfg.delay_bin_s > 0.0) ? obsCfg.delay_bin_s : result.delay_resolution_s;
        std::map<int, Complex> bins;
        for (const auto& p : paths) {
            int idx = static_cast<int>(std::floor(p.delay_s / binS));
            bins[idx] = bins[idx] + p.alpha;
        }
        for (auto& [idx, alpha] : bins) {
            CIRTap tap;
            tap.delay_s = (idx + 0.5) * binS;
            tap.amplitude_real = alpha.re;
            tap.amplitude_imag = alpha.im;
            tap.power_linear = alpha.NormSq();
            result.ideal_delta_cir.taps.push_back(tap);
        }
        result.ideal_delta_cir.coherent = true;
    }

    result.valid = true;
    return result;
}

// ═══════════════════════════════════════════════════════
// v9 C-4: 逐频点EM重评估CFR
// ═══════════════════════════════════════════════════════
BroadbandChannelResult BuildBroadbandCFR_FrequencySweep(
    const EMPathResultSet& pathResultsAtCenter,
    const FrequencySweepConfig& sweepCfg,
    const ChannelObservationConfig& obsCfg,
    const MaterialDatabase* matDb,
    const Scene* scene)
{
    // v9 StageE: frequency_sweep_em not yet implemented — return unsupported
    BroadbandChannelResult result;
    result.valid = false;
    (void)pathResultsAtCenter; (void)sweepCfg; (void)obsCfg; (void)matDb; (void)scene;
    return result;
    // Original implementation preserved below for future activation
    /*
    BroadbandChannelResult result;
    if (!sweepCfg.enabled || sweepCfg.point_count < 1) {
        result.valid = false;
        return result;
    }

    result.center_frequency_hz = sweepCfg.center_hz;
    result.bandwidth_hz = sweepCfg.bandwidth_hz;
    result.window_type = obsCfg.window_type;
    result.delay_resolution_s = 1.0 / sweepCfg.bandwidth_hz;
    result.max_unambiguous_delay_s = sweepCfg.point_count / sweepCfg.bandwidth_hz;

    double fStart = sweepCfg.center_hz - sweepCfg.bandwidth_hz * 0.5;
    double fStop  = sweepCfg.center_hz + sweepCfg.bandwidth_hz * 0.5;
    int N = sweepCfg.point_count;

    result.frequencies_hz.resize(N);
    result.cfr.resize(N);

    // 从中心频率结果提取路径几何信息
    struct PathGeom { double delay_s; double totalLen; int nTrans; std::vector<int> faceIds; };
    std::vector<PathGeom> paths;
    for (const auto& item : pathResultsAtCenter.results) {
        if (!item.valid) continue;
        PathGeom pg;
        pg.delay_s = item.delay_s;
        pg.totalLen = item.total_length_m;
        paths.push_back(pg);
    }

    for (int i = 0; i < N; ++i) {
        double f = (sweepCfg.spacing == "log")
            ? fStart * std::pow(fStop / fStart, (double)i / std::max(1, N - 1))
            : fStart + (fStop - fStart) * i / std::max(1, N - 1);
        result.frequencies_hz[i] = f;

        // 频率相关的波数 k₀(f) = 2πf/c₀
        double k0 = kTwoPi * f / kC0;

        Complex H(0.0, 0.0);
        for (size_t pi = 0; pi < paths.size(); ++pi) {
            // 路径复幅度在中心频率的值 (从中心频率结果提取)
            const auto& item = pathResultsAtCenter.results[pi];
            Complex alpha_c(item.amplitude_real * std::cos(item.phase_rad),
                            item.amplitude_real * std::sin(item.phase_rad));

            // 频率相关的传播相位: φ(f) = -k₀(f)·n·d
            // 其中 k₀(f)=2πf/c₀, n从材料DB查询
            double propPhase = -k0 * item.total_length_m;
            // 介质折射率: 从材料DB查询 (C-5完善)
            double nEff = 1.0;
            if (matDb && scene && !matDb->empty()) {
                nEff = 1.0; // 自由空间为主, 介质路径待C-5细化
            }

            double totalPhase = -k0 * nEff * item.total_length_m;
            Complex alpha_f = alpha_c * Complex(std::cos(totalPhase), std::sin(totalPhase));

            H = H + alpha_f;
        }

        result.cfr[i].frequency_hz = f;
        result.cfr[i].H_real = H.re;
        result.cfr[i].H_imag = H.im;
        result.cfr[i].magnitude = H.Norm();
        result.cfr[i].phase_rad = H.Arg();
        result.cfr[i].power_dB = (H.NormSq() > 0.0) ? (10.0 * std::log10(H.NormSq())) : -300.0;
    }

    // IFFT → 可观测CIR (复用C-3的IFFT逻辑)
    if (obsCfg.export_observed_cir_ifft && N >= 2) {
        std::vector<double> window(N, 1.0);
        if (obsCfg.window_type == "hann") {
            for (int i = 0; i < N; ++i) window[i] = 0.5 * (1.0 - std::cos(kTwoPi * i / (N - 1)));
        } else if (obsCfg.window_type == "hamming") {
            for (int i = 0; i < N; ++i) window[i] = 0.54 - 0.46 * std::cos(kTwoPi * i / (N - 1));
        }

        std::vector<Complex> Hw(N);
        for (int i = 0; i < N; ++i)
            Hw[i] = Complex(result.cfr[i].H_real * window[i], result.cfr[i].H_imag * window[i]);

        int nBins = std::min(N, 2048);
        for (int k = 0; k < nBins; ++k) {
            Complex sum(0.0, 0.0);
            for (int i = 0; i < N; ++i) {
                double phase = kTwoPi * i * k / N;
                sum = sum + Hw[i] * Complex(std::cos(phase), std::sin(phase));
            }
            CIRTap tap;
            tap.delay_s = k * result.delay_resolution_s;
            tap.amplitude_real = sum.re / N;
            tap.amplitude_imag = sum.im / N;
            tap.power_linear = sum.NormSq() / (N * N);
            result.observed_cir.taps.push_back(tap);

            PDPTap ptap;
            ptap.delay_s = tap.delay_s;
            ptap.power_linear = tap.power_linear;
            result.observed_pdp.taps.push_back(ptap);
        }
        result.observed_cir.coherent = true;
    }

    result.valid = true;
    return result;
    */
}

// ═══════════════════════════════════════════════════════
// v9 C-8: 信道统计指标
// ═══════════════════════════════════════════════════════
ChannelStatisticsResult ComputeChannelStatistics(const EMPathResultSet& pathResults) {
    ChannelStatisticsResult r;

    // 筛选有效路径并排序 (按功率)
    struct P { double pwr; double delay; };
    std::vector<P> valid;
    for (auto& item : pathResults.results) {
        if (!item.valid || item.power_linear <= 0.0) continue;
        valid.push_back({item.power_linear, item.delay_s});
    }
    if (valid.empty()) return r;

    r.path_count = static_cast<int>(valid.size());

    // 总功率
    for (auto& p : valid) r.total_power_linear += p.pwr;

    // 找LOS路径 (最小时延 = LOS)
    double minDelay = valid[0].delay;
    double losPower = 0.0;
    for (auto& p : valid) {
        if (p.delay < minDelay) minDelay = p.delay;
    }
    // LOS = 第一个到达的路径的功率
    for (auto& p : valid) {
        if (std::fabs(p.delay - minDelay) < 1e-12) {
            losPower = p.pwr;
            break;
        }
    }

    // 散射功率 = 总功率 - LOS功率
    double scatPower = r.total_power_linear - losPower;
    if (scatPower > 1e-30 && losPower > 0.0) {
        r.k_factor_dB = 10.0 * std::log10(losPower / scatPower);
    } else if (scatPower <= 1e-30) {
        r.k_factor_dB = 40.0; // 纯LOS (上限)
    }

    // 平均附加时延: τ_mean = Σ(P_i·τ_i) / ΣP_i - τ_LOS
    double sumPwrTau = 0.0;
    for (auto& p : valid) sumPwrTau += p.pwr * p.delay;
    r.mean_excess_delay_s = (sumPwrTau / r.total_power_linear) - minDelay;
    if (r.mean_excess_delay_s < 0.0) r.mean_excess_delay_s = 0.0;

    // RMS delay spread: τ_rms = sqrt(Σ(P_i·(τ_i-τ_mean)²) / ΣP_i)
    double sumPwrTauSq = 0.0;
    double tauMean = r.mean_excess_delay_s + minDelay; // absolute mean delay
    for (auto& p : valid) {
        double dTau = p.delay - tauMean;
        sumPwrTauSq += p.pwr * dTau * dTau;
    }
    r.rms_delay_spread_s = std::sqrt(sumPwrTauSq / r.total_power_linear);

    r.valid = true;
    return r;
}

} // namespace rt
