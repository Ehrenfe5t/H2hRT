// 文件目标：
// - 实现模块6批次9的信道结果导出逻辑。
//
// 主要功能：
// - 输出 CIR / PDP / APS / Statistics JSON 文件；
// - 保证模块6结构化结果文件完整导出目标可检查；
// - 为后续脚本直接消费提供基础格式。

#include "ExportChannel.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出信道结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportChannel(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/channel";
    const std::string path = dir + "/channel_summary.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n"
         << "  \"export_schema_version\": \"" << bundle.export_schema_version << "\",\n"
         << "  \"export_purpose\": \"" << context.export_purpose << "\",\n"
         << "  \"primary_input_source\": \"" << context.primary_input_source << "\",\n"
         << "  \"cir_taps\": " << context.precise_result->cir.taps.size() << ",\n"
         << "  \"pdp_taps\": " << context.precise_result->pdp.taps.size() << ",\n"
         << "  \"aps_entries\": " << context.precise_result->aps.entries.size() << ",\n"
         << "  \"total_power_linear\": " << context.precise_result->statistics.total_power_linear << ",\n"
         << "  \"mean_abs_phase_rad\": " << context.precise_result->statistics.mean_abs_phase_rad << ",\n"
         << "  \"transmission_path_count\": " << context.precise_result->statistics.transmission_path_count << "\n"
         << "}\n";

    if (!WriteTextFile(path, json.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    ++bundle.exported_json_file_count;

    // v9 StageE: 完整宽带CFR/CIR/PDP导出
    if (context.broadband_result && context.broadband_result->valid) {
        const auto& br = *context.broadband_result;

        // Helper: write JSON + CSV pair
        auto writePair = [&](const std::string& name, const std::string& jsonBody,
                              const std::string& csvHeader, const std::string& csvBody) {
            std::string jp = dir + "/" + name + ".json";
            std::string cp = dir + "/" + name + ".csv";
            WriteTextFile(jp, jsonBody); bundle.exported_files.push_back(jp);
            WriteTextFile(cp, csvHeader + "\n" + csvBody); bundle.exported_files.push_back(cp);
        };

        // ── cfr_sampled ──
        {
            std::ostringstream json, csvH, csvB;
            json << "{\"center_hz\":" << br.center_frequency_hz
                 << ",\"bandwidth_hz\":" << br.bandwidth_hz
                 << ",\"freq_count\":" << br.cfr.size()
                 << ",\"H\":[";
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
            writePair("cfr_sampled", json.str(), csvH.str(), csvB.str());
        }

        // ── cir_ideal_delta ──
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
            writePair("cir_ideal_delta", json.str(), csvH.str(), csvB.str());
        }

        // ── cir_observed_ifft ──
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
            writePair("cir_observed_ifft", json.str(), csvH.str(), csvB.str());
        }

        // ── pdp_coherent + pdp_incoherent ──
        {
            std::ostringstream jc, ccsvH, ccsvB, ji, icsvH, icsvB;
            jc << "{\"type\":\"coherent\",\"taps\":["; ji << "{\"type\":\"incoherent\",\"taps\":[";
            ccsvH << "delay_s,power_linear"; icsvH << "delay_s,power_linear";
            const auto& taps = br.observed_pdp.taps;
            for (size_t i = 0; i < taps.size(); ++i) {
                if (i > 0) {
                    jc << ","; ji << ","; ccsvB << "\n"; icsvB << "\n";
                }
                jc << "[" << taps[i].delay_s << "," << taps[i].coherent_power_linear << "]";
                ji << "[" << taps[i].delay_s << "," << taps[i].incoherent_power_linear << "]";
                ccsvB << taps[i].delay_s << "," << taps[i].coherent_power_linear;
                icsvB << taps[i].delay_s << "," << taps[i].incoherent_power_linear;
            }
            jc << "]}"; ji << "]}";
            writePair("pdp_coherent", jc.str(), ccsvH.str(), ccsvB.str());
            writePair("pdp_incoherent", ji.str(), icsvH.str(), icsvB.str());
        }

        // ── wideband_metadata ──
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
            WriteTextFile(mp, m.str());
            bundle.exported_files.push_back(mp);
        }
    }

    return true;
}

} // namespace rt
