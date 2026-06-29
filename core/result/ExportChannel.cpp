// 文件目标：
// - 导出信道结果文件：CIR, PDP, APS, channel statistics, XPR stats, MEG。
// - v11.3: 从 channel_summary.json 扩展为完整论文数据产品。
//
// 主要功能：
// - 输出 cir.json, pdp.json, aps_2d_grid.json, xpr_stats.json, channel_stats.json, meg.json
// - 保留 channel_summary.json 作为快速摘要
// - 支持宽带 CFR/CIR/PDP 导出（条件触发）

#include "ExportChannel.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出信道结果文件（v11.3 扩展版）。
/// </summary>
bool ExportChannel(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/channel";
    if (!EnsureResultDirectory(dir))
        return false;

    const EMAggregateResult& R = *context.precise_result;
    bool allWritesSucceeded = true;

    auto writeFile = [&](const std::string& path, const std::string& body) {
        if (!WriteTextFile(path, body)) {
            allWritesSucceeded = false;
            return false;
        }
        bundle.exported_files.push_back(path);
        return true;
    };

    // ── Helper: write a JSON+CSV pair from two string builders ──
    auto writePair = [&](const std::string& name, const std::string& jsonBody,
                          const std::string& csvHeader, const std::string& csvBody) {
        std::string jp = dir + "/" + name + ".json";
        std::string cp = dir + "/" + name + ".csv";
        writeFile(jp, jsonBody);
        writeFile(cp, csvHeader + "\n" + csvBody);
    };

    // ═══════════════════════════════════════════════════════════════
    // 1. channel_summary.json (保留扩展版)
    // ═══════════════════════════════════════════════════════════════
    {
        std::ostringstream json;
        json << "{\n"
             << "  \"export_schema_version\": \"" << bundle.export_schema_version << "\",\n"
             << "  \"export_purpose\": \"" << context.export_purpose << "\",\n"
             << "  \"primary_input_source\": \"" << context.primary_input_source << "\",\n"
             << "  \"cir_taps\": " << R.cir.taps.size() << ",\n"
             << "  \"pdp_taps\": " << R.pdp.taps.size() << ",\n"
             << "  \"aps_entries\": " << R.aps.entries.size() << ",\n"
             << "  \"total_power_linear\": " << R.statistics.total_power_linear << ",\n"
             << "  \"los_power_linear\": " << R.statistics.los_power_linear << ",\n"
             << "  \"nlos_power_linear\": " << R.statistics.nlos_power_linear << ",\n"
             << "  \"strongest_path_power_linear\": " << R.statistics.strongest_path_power_linear << ",\n"
             << "  \"mean_delay_s\": " << R.statistics.mean_delay_s << ",\n"
             << "  \"rms_delay_spread_s\": " << R.statistics.rms_delay_spread_s << ",\n"
             << "  \"k_factor_dB\": " << R.statistics.k_factor_dB << ",\n"
             << "  \"effective_path_count\": " << R.statistics.effective_path_count << ",\n"
             << "  \"transmission_path_count\": " << R.statistics.transmission_path_count << ",\n"
             << "  \"xpr_mean_dB\": " << R.xpr_stats.mean_dB << ",\n"
             << "  \"xpr_median_dB\": " << R.xpr_stats.median_dB << ",\n"
             << "  \"meg_dB\": " << R.meg_dB << "\n"
             << "}\n";
        std::string path = dir + "/channel_summary.json";
        writeFile(path, json.str());
        ++bundle.exported_json_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 2. cir.json / cir.csv
    // ═══════════════════════════════════════════════════════════════
    {
        std::ostringstream json, csvH, csvB;
        json << "{\"coherent\":" << (R.cir.coherent ? "true" : "false")
             << ",\"tap_count\":" << R.cir.taps.size() << ",\"taps\":[";
        csvH << "delay_s,amplitude_real,amplitude_imag,power_linear";
        const auto& taps = R.cir.taps;
        for (size_t i = 0; i < taps.size(); ++i) {
            if (i > 0) { json << ","; csvB << "\n"; }
            json << "[" << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                 << taps[i].amplitude_imag << "," << taps[i].power_linear << "]";
            csvB << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                 << taps[i].amplitude_imag << "," << taps[i].power_linear;
        }
        json << "]}";
        writePair("cir", json.str(), csvH.str(), csvB.str());
        ++bundle.exported_json_file_count;
        ++bundle.exported_csv_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 3. pdp.json / pdp.csv (per-path taps, with coherent/incoherent if available)
    // ═══════════════════════════════════════════════════════════════
    {
        std::ostringstream json, csvH, csvB;
        json << "{\"tap_count\":" << R.pdp.taps.size() << ",\"taps\":[";
        csvH << "delay_s,power_linear,coherent_power_linear,incoherent_power_linear";
        const auto& taps = R.pdp.taps;
        bool hasBins = false;
        for (size_t i = 0; i < taps.size(); ++i) {
            if (i > 0) { json << ","; csvB << "\n"; }
            json << "[" << taps[i].delay_s << "," << taps[i].power_linear << ","
                 << taps[i].coherent_power_linear << "," << taps[i].incoherent_power_linear << "]";
            csvB << taps[i].delay_s << "," << taps[i].power_linear << ","
                 << taps[i].coherent_power_linear << "," << taps[i].incoherent_power_linear;
            if (taps[i].coherent_power_linear > 0.0 || taps[i].incoherent_power_linear > 0.0)
                hasBins = true;
        }
        json << "],\"has_coherent_incoherent_split\":" << (hasBins ? "true" : "false") << "}";
        writePair("pdp", json.str(), csvH.str(), csvB.str());
        ++bundle.exported_json_file_count;
        ++bundle.exported_csv_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 4. aps_2d_grid.json / aps_2d_grid.csv (2D theta-phi grid)
    // ═══════════════════════════════════════════════════════════════
    if (R.aps.has_2d_grid && !R.aps.power_grid_linear.empty()) {
        const APSResult& aps = R.aps;
        // JSON: full grid with metadata
        {
            std::ostringstream json;
            json << "{\"n_theta\":" << aps.n_theta << ",\"n_phi\":" << aps.n_phi
                 << ",\"theta_min_deg\":" << aps.theta_min_deg
                 << ",\"theta_max_deg\":" << aps.theta_max_deg
                 << ",\"theta_step_deg\":" << aps.theta_step_deg
                 << ",\"phi_min_deg\":" << aps.phi_min_deg
                 << ",\"phi_max_deg\":" << aps.phi_max_deg
                 << ",\"phi_step_deg\":" << aps.phi_step_deg
                  << ",\"grid_semantics\":\"integrated_bin_power_watts\""
                  << ",\"observed_grid_linear\":[";
            for (int i = 0; i < aps.n_theta; ++i) {
                if (i > 0) json << ",";
                json << "\n  [";
                for (int j = 0; j < aps.n_phi; ++j) {
                    if (j > 0) json << ",";
                    json << aps.power_grid_linear[aps.GridIndex(i, j)];
                }
                json << "]";
            }
            json << "\n],\n  \"observed_grid_dB\":[";
            for (int i = 0; i < aps.n_theta; ++i) {
                if (i > 0) json << ",";
                json << "\n  [";
                for (int j = 0; j < aps.n_phi; ++j) {
                    if (j > 0) json << ",";
                    json << aps.power_grid_dB[aps.GridIndex(i, j)];
                }
                json << "]";
            }
            json << "\n],\n  \"incident_grid_linear\":[";
            for (int i = 0; i < aps.n_theta; ++i) {
                if (i > 0) json << ",";
                json << "\n  [";
                for (int j = 0; j < aps.n_phi; ++j) {
                    if (j > 0) json << ",";
                    json << aps.incident_power_grid_linear[aps.GridIndex(i, j)];
                }
                json << "]";
            }
            json << "\n],\n  \"incident_grid_dB\":[";
            for (int i = 0; i < aps.n_theta; ++i) {
                if (i > 0) json << ",";
                json << "\n  [";
                for (int j = 0; j < aps.n_phi; ++j) {
                    if (j > 0) json << ",";
                    json << aps.incident_power_grid_dB[aps.GridIndex(i, j)];
                }
                json << "]";
            }
            json << "\n]}\n";
            std::string jp = dir + "/aps_2d_grid.json";
            writeFile(jp, json.str());
            ++bundle.exported_json_file_count;
        }
        // CSV: theta_deg, phi_deg, power_linear, power_dB
        {
            std::ostringstream csv;
            csv << "theta_deg,phi_deg,observed_power_linear,observed_power_dB,incident_power_linear,incident_power_dB\n";
            for (int i = 0; i < aps.n_theta; ++i) {
                double thetaDeg = aps.theta_min_deg + (i + 0.5) * aps.theta_step_deg;
                for (int j = 0; j < aps.n_phi; ++j) {
                    double phiDeg = aps.phi_min_deg + (j + 0.5) * aps.phi_step_deg;
                    int idx = aps.GridIndex(i, j);
                    csv << thetaDeg << "," << phiDeg << ","
                        << aps.power_grid_linear[idx] << ","
                        << aps.power_grid_dB[idx] << ","
                        << aps.incident_power_grid_linear[idx] << ","
                        << aps.incident_power_grid_dB[idx] << "\n";
                }
            }
            std::string cp = dir + "/aps_2d_grid.csv";
            writeFile(cp, csv.str());
            ++bundle.exported_csv_file_count;
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // 5. xpr_stats.json
    // ═══════════════════════════════════════════════════════════════
    {
        const XPRStatistics& xs = R.xpr_stats;
        std::ostringstream json;
        json << "{\n"
             << "  \"valid_path_count\": " << xs.valid_path_count << ",\n"
             << "  \"mean_dB\": " << xs.mean_dB << ",\n"
             << "  \"median_dB\": " << xs.median_dB << ",\n"
             << "  \"p10_dB\": " << xs.p10_dB << ",\n"
             << "  \"p90_dB\": " << xs.p90_dB << ",\n"
             << "  \"power_weighted_mean_dB\": " << xs.power_weighted_mean_dB << ",\n"
             << "  \"aggregate_xpr_dB\": " << xs.aggregate_xpr_dB << ",\n"
             << "  \"pure_co_path_count\": " << xs.pure_co_path_count << ",\n"
             << "  \"pure_cross_path_count\": " << xs.pure_cross_path_count << ",\n"
             << "  \"zero_polarized_power_count\": " << xs.zero_polarized_power_count << ",\n"
             << "  \"censor_limit_dB\": " << xs.censor_limit_dB << ",\n"
             << "  \"min_dB\": " << xs.min_dB << ",\n"
             << "  \"max_dB\": " << xs.max_dB << ",\n"
             << "  \"cdf_values_dB\": [";
        const auto& vals = xs.xpr_values_dB;
        for (size_t i = 0; i < vals.size(); ++i) {
            if (i > 0) json << ",";
            json << vals[i];
        }
        json << "]\n}\n";
        std::string jp = dir + "/xpr_stats.json";
        writeFile(jp, json.str());
        ++bundle.exported_json_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 6. channel_stats.json
    // ═══════════════════════════════════════════════════════════════
    {
        const ChannelStatistics& cs = R.statistics;
        std::ostringstream json;
        json << "{\n"
             << "  \"valid_path_count\": " << cs.valid_path_count << ",\n"
             << "  \"total_power_linear\": " << cs.total_power_linear << ",\n"
             << "  \"los_power_linear\": " << cs.los_power_linear << ",\n"
             << "  \"nlos_power_linear\": " << cs.nlos_power_linear << ",\n"
             << "  \"strongest_path_power_linear\": " << cs.strongest_path_power_linear << ",\n"
             << "  \"strongest_path_delay_s\": " << cs.strongest_path_delay_s << ",\n"
             << "  \"mean_delay_s\": " << cs.mean_delay_s << ",\n"
             << "  \"power_weighted_mean_delay_s\": " << cs.power_weighted_mean_delay_s << ",\n"
             << "  \"rms_delay_spread_s\": " << cs.rms_delay_spread_s << ",\n"
             << "  \"k_factor_dB\": " << cs.k_factor_dB << ",\n"
             << "  \"effective_path_count\": " << cs.effective_path_count << ",\n"
             << "  \"transmission_path_count\": " << cs.transmission_path_count << "\n"
             << "}\n";
        std::string jp = dir + "/channel_stats.json";
        writeFile(jp, json.str());
        ++bundle.exported_json_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 7. meg.json
    // ═══════════════════════════════════════════════════════════════
    {
        std::ostringstream json;
        json << "{\n"
             << "  \"definition\": \"gain-weighted incident APS; bins contain integrated power\",\n"
             << "  \"meg_linear\": " << R.meg_linear << ",\n"
             << "  \"meg_dB\": " << R.meg_dB << "\n"
             << "}\n";
        std::string jp = dir + "/meg.json";
        writeFile(jp, json.str());
        ++bundle.exported_json_file_count;
    }

    // ═══════════════════════════════════════════════════════════════
    // 8. 宽带导出 (保留 v9 代码，条件触发)
    // ═══════════════════════════════════════════════════════════════
    if (context.broadband_result && context.broadband_result->valid) {
        const auto& br = *context.broadband_result;

        auto writeBroadbandPair = [&](const std::string& name, const std::string& jsonBody,
                                       const std::string& csvHeader, const std::string& csvBody) {
            std::string jp = dir + "/" + name + ".json";
            std::string cp = dir + "/" + name + ".csv";
            writeFile(jp, jsonBody);
            writeFile(cp, csvHeader + "\n" + csvBody);
        };

        // cfr_sampled
        {
            std::ostringstream json, csvH, csvB;
            json << "{\"center_hz\":" << br.center_frequency_hz
                 << ",\"bandwidth_hz\":" << br.bandwidth_hz
                 << ",\"freq_count\":" << br.cfr.size() << ",\"H\":[";
            csvH << "freq_hz,H_real,H_imag,magnitude,phase_rad,power_dB";
            for (size_t i = 0; i < br.cfr.size(); ++i) {
                if (i > 0) { json << ","; csvB << "\n"; }
                json << "[" << br.frequencies_hz[i] << "," << br.cfr[i].H_real << ","
                     << br.cfr[i].H_imag << "," << br.cfr[i].magnitude << ","
                     << br.cfr[i].phase_rad << "," << br.cfr[i].power_dB << "]";
                csvB << br.frequencies_hz[i] << "," << br.cfr[i].H_real << ","
                     << br.cfr[i].H_imag << "," << br.cfr[i].magnitude << ","
                     << br.cfr[i].phase_rad << "," << br.cfr[i].power_dB;
            }
            json << "]}";
            writeBroadbandPair("cfr_sampled", json.str(), csvH.str(), csvB.str());
        }

        // cir_ideal_delta
        {
            std::ostringstream json, csvH, csvB;
            json << "{\"coherent\":true,\"taps\":[";
            csvH << "delay_s,amplitude_real,amplitude_imag,power_linear";
            const auto& taps = br.ideal_delta_cir.taps;
            for (size_t i = 0; i < taps.size(); ++i) {
                if (i > 0) { json << ","; csvB << "\n"; }
                json << "[" << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                     << taps[i].amplitude_imag << "," << taps[i].power_linear << "]";
                csvB << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                     << taps[i].amplitude_imag << "," << taps[i].power_linear;
            }
            json << "]}";
            writeBroadbandPair("cir_ideal_delta", json.str(), csvH.str(), csvB.str());
        }

        // cir_observed_ifft
        {
            std::ostringstream json, csvH, csvB;
            json << "{\"coherent\":" << (br.observed_cir.coherent ? "true" : "false")
                 << ",\"window\":\"" << br.window_type << "\",\"taps\":[";
            csvH << "delay_s,amplitude_real,amplitude_imag,power_linear";
            const auto& taps = br.observed_cir.taps;
            for (size_t i = 0; i < taps.size(); ++i) {
                if (i > 0) { json << ","; csvB << "\n"; }
                json << "[" << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                     << taps[i].amplitude_imag << "," << taps[i].power_linear << "]";
                csvB << taps[i].delay_s << "," << taps[i].amplitude_real << ","
                     << taps[i].amplitude_imag << "," << taps[i].power_linear;
            }
            json << "]}";
            writeBroadbandPair("cir_observed_ifft", json.str(), csvH.str(), csvB.str());
        }

        // pdp_coherent + pdp_incoherent
        {
            std::ostringstream jc, ccsvH, ccsvB, ji, icsvH, icsvB;
            jc << "{\"type\":\"coherent\",\"taps\":["; ji << "{\"type\":\"incoherent\",\"taps\":[";
            ccsvH << "delay_s,power_linear"; icsvH << "delay_s,power_linear";
            const auto& taps = br.observed_pdp.taps;
            for (size_t i = 0; i < taps.size(); ++i) {
                if (i > 0) { jc << ","; ji << ","; ccsvB << "\n"; icsvB << "\n"; }
                jc << "[" << taps[i].delay_s << "," << taps[i].coherent_power_linear << "]";
                ji << "[" << taps[i].delay_s << "," << taps[i].incoherent_power_linear << "]";
                ccsvB << taps[i].delay_s << "," << taps[i].coherent_power_linear;
                icsvB << taps[i].delay_s << "," << taps[i].incoherent_power_linear;
            }
            jc << "]}"; ji << "]}";
            writeBroadbandPair("pdp_coherent", jc.str(), ccsvH.str(), ccsvB.str());
            writeBroadbandPair("pdp_incoherent", ji.str(), icsvH.str(), icsvB.str());
        }

        // wideband_metadata
        {
            std::ostringstream m;
            m << "{\n  \"mode\": \"fixed_gain\",\n"
              << "  \"center_hz\": " << br.center_frequency_hz << ",\n"
              << "  \"bandwidth_hz\": " << br.bandwidth_hz << ",\n"
              << "  \"freq_points\": " << br.cfr.size() << ",\n"
              << "  \"delay_resolution_s\": " << br.delay_resolution_s << ",\n"
              << "  \"window_type\": \"" << br.window_type << "\",\n"
              << "  \"ifft_cir_taps\": " << br.observed_cir.taps.size() << ",\n"
              << "  \"ideal_cir_taps\": " << br.ideal_delta_cir.taps.size() << ",\n"
              << "  \"pdp_taps\": " << br.observed_pdp.taps.size() << ",\n"
              << "  \"frequency_sweep_em_supported\": false,\n"
              << "  \"frequency_sweep_em_note\": \"not yet implemented; fixed_gain approximation used\"\n"
              << "}\n";
            std::string mp = dir + "/wideband_metadata.json";
            writeFile(mp, m.str());
        }
    }

    return allWritesSucceeded;
}

} // namespace rt
