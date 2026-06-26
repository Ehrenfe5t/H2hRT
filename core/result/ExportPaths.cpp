// 文件目标：
// - 实现模块6批次9的路径结果导出逻辑。
//
// 主要功能：
// - 输出路径级 JSON 与 CSV 文件；
// - 保持结构清晰、字段稳定；
// - 为路径人工核查和模块6闭环提供基础文件。

#include "ExportPaths.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace {

// v9 B-8: Guard against NaN/inf in JSON output (replace with valid numeric sentinels)
double SafeJsonDouble(double v) {
    if (std::isnan(v)) return 0.0;
    if (std::isinf(v)) return (v > 0.0) ? 300.0 : -300.0;
    return v;
}

} // namespace

namespace rt {

/// <summary>
/// 导出路径级结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportPaths(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string jsonPath = context.export_root_directory + "/paths/precise_paths.json";
    const std::string csvPath = context.export_root_directory + "/paths/precise_paths.csv";
    const std::string manifestPath = context.export_root_directory + "/paths/path_manifest.json";
    if (!EnsureResultDirectory(context.export_root_directory + "/paths"))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n  \"path_count\": " << context.precise_result->path_results.results.size()
          << ",\n  \"primary_input_source\": \"" << context.primary_input_source << "\""
          << ",\n  \"real_chain_enabled\": " << (context.real_chain_enabled ? "true" : "false")
          << ",\n  \"export_purpose\": \"" << context.export_purpose << "\""
          << ",\n  \"export_schema_version\": \"" << bundle.export_schema_version << "\""
          << ",\n  \"handoff_view_name\": \"" << bundle.export_view_name << "\""
          << ",\n  \"paths\": [\n";
    for (std::size_t i = 0; i < context.precise_result->path_results.results.size(); ++i)
    {
        const EMPathResult& item = context.precise_result->path_results.results[i];
        const GeometricPath* sourcePath = nullptr;
        if (context.search_result != nullptr)
        {
            for (const GeometricPath& candidatePath : context.search_result->path_set.paths)
            {
                if (candidatePath.path_signature == item.source_path_signature || item.source_path_signature == 0)
                {
                    sourcePath = &candidatePath;
                    break;
                }
            }
        }

        json << "    { \"path_id\": " << item.path_id
             << ", \"delay_s\": " << item.delay_s
             << ", \"phase_rad\": " << item.phase_rad
             << ", \"power_linear\": " << item.power_linear
             << ", \"free_space_loss_db\": " << item.free_space_loss_db
             << ", \"polarization_magnitude\": " << item.polarization_magnitude
             << ", \"polarization_vector\": [" << item.polarization_vector.x << "," << item.polarization_vector.y << "," << item.polarization_vector.z << "]"
             << ", \"polarization_imag\": [" << item.polarization_imag.x << "," << item.polarization_imag.y << "," << item.polarization_imag.z << "]"
             << ", \"is_los\": " << (item.is_los ? "true" : "false")
             << ", \"contains_transmission\": " << (item.contains_transmission ? "true" : "false")
             << ", \"transmission_semantic_consumed\": " << (item.transmission_semantic_consumed ? "true" : "false")
             << ", \"first_transmission_medium_in_id\": " << item.first_transmission_medium_in_id
             << ", \"first_transmission_medium_out_id\": " << item.first_transmission_medium_out_id
             << ", \"aod_theta_deg\": " << item.aod_theta_deg << ", \"aod_phi_deg\": " << item.aod_phi_deg
             << ", \"aoa_theta_deg\": " << item.aoa_theta_deg << ", \"aoa_phi_deg\": " << item.aoa_phi_deg
             << ", \"co_pol_power_linear\": " << SafeJsonDouble(item.co_pol_power_linear)
             << ", \"cross_pol_power_linear\": " << SafeJsonDouble(item.cross_pol_power_linear)
             << ", \"xpr_dB\": " << SafeJsonDouble(item.xpr_dB)
             << ", \"source_tag\": \"" << item.source_tag << "\""
             << ", \"tx_antenna_id\": \"" << item.tx_antenna_id << "\""
             << ", \"tx_antenna_source_type\": \"" << item.tx_antenna_source_type << "\""
             << ", \"rx_antenna_id\": \"" << item.rx_antenna_id << "\""
             << ", \"rx_antenna_source_type\": \"" << item.rx_antenna_source_type << "\""
             << ", \"source_path_signature\": \"0x" << std::hex << item.source_path_signature << std::dec << "\""
             << ", \"geometry_residual\": " << (sourcePath ? sourcePath->geometry_residual : 0.0)
             << ", \"reflection_residual_m\": " << (sourcePath ? sourcePath->reflection_residual_m : 0.0)
             << ", \"max_snell_residual\": " << (sourcePath ? sourcePath->max_snell_residual : 0.0)
             << ", \"max_keller_residual\": " << (sourcePath ? sourcePath->max_keller_residual : 0.0)
             << ", \"residual_reject_reason\": \"" << (sourcePath ? sourcePath->residual_reject_reason : std::string()) << "\""
             << ", \"interaction_types\": [";

        if (sourcePath != nullptr)
        {
            for (std::size_t nodeIndex = 0; nodeIndex < sourcePath->nodes.size(); ++nodeIndex)
            {
                const PathNode& node = sourcePath->nodes[nodeIndex];
                json << static_cast<int>(node.interaction_type);
                if (nodeIndex + 1U < sourcePath->nodes.size())
                {
                    json << ", ";
                }
            }
        }
        json << "]"
             << ", \"geometry_nodes\": [";

        if (sourcePath != nullptr)
        {
            for (std::size_t nodeIndex = 0; nodeIndex < sourcePath->nodes.size(); ++nodeIndex)
            {
                const PathNode& node = sourcePath->nodes[nodeIndex];
                json << "{ \"interaction_type\": " << static_cast<int>(node.interaction_type)
                     << ", \"object_id\": " << node.object_id
                     << ", \"face_id\": " << node.face_id
                     << ", \"wedge_id\": " << node.wedge_id
                     << ", \"x\": " << node.point.x
                     << ", \"y\": " << node.point.y
                     << ", \"z\": " << node.point.z
                     // v9 StageB: medium semantics
                     << ", \"medium_in_id\": " << node.medium_in_id
                     << ", \"medium_out_id\": " << node.medium_out_id
                     << ", \"front_medium_id\": " << node.front_medium_id
                     << ", \"back_medium_id\": " << node.back_medium_id
                     << ", \"entered_from_front_side\": " << (node.entered_from_front_side ? "true" : "false")
                     << ", \"transmission_semantic_complete\": " << (node.transmission_semantic_complete ? "true" : "false")
                     // v9 StageB: direction vectors
                     << ", \"incident_dx\": " << node.incident_direction.x
                     << ", \"incident_dy\": " << node.incident_direction.y
                     << ", \"incident_dz\": " << node.incident_direction.z
                     << ", \"direction_dx\": " << node.direction.x
                     << ", \"direction_dy\": " << node.direction.y
                     << ", \"direction_dz\": " << node.direction.z
                     << ", \"normal_nx\": " << node.surface_normal.x
                     << ", \"normal_ny\": " << node.surface_normal.y
                     << ", \"normal_nz\": " << node.surface_normal.z
                     << ", \"segment_length\": " << node.segment_length_from_previous
                     // v9 StageB: Snell diagnostics
                     << ", \"snell_residual\": " << node.snell_residual
                     << ", \"snell_theta_i_rad\": " << node.snell_theta_i_rad
                     << ", \"snell_theta_t_rad\": " << node.snell_theta_t_rad
                     << ", \"snell_tir\": " << (node.snell_tir ? "true" : "false");
                // v9 StageB: Diffraction diagnostics (only for Diffraction nodes)
                if (node.interaction_type == InteractionType::Diffraction) {
                    const auto& dd = node.diffraction_diag;
                    json << ", \"diffraction_diag\": {"
                         << "\"edge_t\": " << dd.edge_parameter_t
                         << ", \"s1\": " << dd.s1
                         << ", \"s2\": " << dd.s2
                         << ", \"keller_residual\": " << dd.keller_residual
                         << ", \"fermat_endpoint_warning\": " << (dd.fermat_endpoint_warning ? "true" : "false")
                         << ", \"vis_from_source\": " << (dd.visibility_from_source ? "true" : "false")
                         << ", \"vis_to_rx\": " << (dd.visibility_to_rx ? "true" : "false")
                         << "}";
                }
                json << " }";
                if (nodeIndex + 1U < sourcePath->nodes.size())
                {
                    json << ", ";
                }
            }
        }
        json << "]"
             << " }";
        if (i + 1U < context.precise_result->path_results.results.size())
        {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n}\n";

    // v7 H16: CSV 标准转义 — 含逗号/引号的字段双引号包裹, 内部引号加倍
    auto csvEscape = [](const std::string& s) -> std::string {
        if (s.find_first_of(",\"\n") == std::string::npos) return s;
        std::string out = "\"";
        for (char c : s) { if (c == '\"') out += "\"\""; else out += c; }
        out += "\""; return out;
    };

    std::ostringstream csv;
    csv << "path_id,delay_s,phase_rad,power_linear,free_space_loss_db,polarization_magnitude,is_los,contains_transmission,transmission_semantic_consumed,first_transmission_medium_in_id,first_transmission_medium_out_id,source_tag,tx_antenna_id,tx_antenna_source_type,rx_antenna_id,rx_antenna_source_type,source_path_signature,geometry_residual,reflection_residual_m,max_snell_residual,max_keller_residual,residual_reject_reason,co_pol_power_linear,cross_pol_power_linear,xpr_dB,node_count\n";
    for (const EMPathResult& item : context.precise_result->path_results.results)
    {
        std::size_t nodeCount = 0;
        const GeometricPath* sourcePath = nullptr;
        if (context.search_result != nullptr)
        {
            for (const GeometricPath& candidatePath : context.search_result->path_set.paths)
            {
                if (candidatePath.path_signature == item.source_path_signature || item.source_path_signature == 0)
                {
                    sourcePath = &candidatePath;
                    nodeCount = candidatePath.nodes.size();
                    break;
                }
            }
        }
        csv << item.path_id << "," << item.delay_s << "," << item.phase_rad << "," << item.power_linear << "," << item.free_space_loss_db << "," << item.polarization_magnitude << ","
            << (item.is_los ? "true" : "false") << ","
            << (item.contains_transmission ? "true" : "false") << ","
            << (item.transmission_semantic_consumed ? "true" : "false") << ","
            << item.first_transmission_medium_in_id << ","
            << item.first_transmission_medium_out_id << ","
            << csvEscape(item.source_tag) << ","
            << csvEscape(item.tx_antenna_id) << ","
            << csvEscape(item.tx_antenna_source_type) << ","
            << csvEscape(item.rx_antenna_id) << ","
            << csvEscape(item.rx_antenna_source_type) << ","
            << item.source_path_signature << ","
            << (sourcePath ? sourcePath->geometry_residual : 0.0) << ","
            << (sourcePath ? sourcePath->reflection_residual_m : 0.0) << ","
            << (sourcePath ? sourcePath->max_snell_residual : 0.0) << ","
            << (sourcePath ? sourcePath->max_keller_residual : 0.0) << ","
            << csvEscape(sourcePath ? sourcePath->residual_reject_reason : std::string()) << ","
            << item.co_pol_power_linear << "," << item.cross_pol_power_linear << "," << item.xpr_dB << ","
            << nodeCount << "\n";
    }

    std::ostringstream manifest;
    manifest << "{\n"
             << "  \"export_purpose\": \"" << context.export_purpose << "\",\n"
             << "  \"handoff_view_name\": \"" << context.handoff_view_name << "\",\n"
             << "  \"export_schema_version\": \"" << bundle.export_schema_version << "\",\n"
             << "  \"primary_input_source\": \"" << context.primary_input_source << "\",\n"
             << "  \"path_file\": \"paths/precise_paths.json\",\n"
             << "  \"csv_file\": \"paths/precise_paths.csv\",\n"
             << "  \"path_count\": " << context.precise_result->path_results.results.size() << ",\n"
             << "  \"source_path_count\": " << (context.search_result != nullptr ? context.search_result->path_set.paths.size() : 0) << "\n"
             << "}\n";

    // v9 StageC: search diagnostics (failure_reason_counts for TIR/Snell validation)
    if (context.search_result != nullptr) {
        std::ostringstream diag;
        diag << "{\n  \"failure_reason_counts\": {";
        bool first = true;
        for (auto& [reason, count] : context.search_result->failure_reason_counts) {
            if (!first) diag << ",";
            diag << "\n    \"" << reason << "\": " << count;
            first = false;
        }
        diag << "\n  },\n  \"generated_state_count\": " << context.search_result->generated_state_count
             << ",\n  \"deduplicated_state_count\": " << context.search_result->deduplicated_state_count
             << ",\n  \"candidate_state_count\": " << context.search_result->candidate_state_count
             << ",\n  \"accepted_state_count\": " << context.search_result->accepted_state_count
             << "\n}\n";
        std::string diagPath = context.export_root_directory + "/paths/search_diagnostics.json";
        WriteTextFile(diagPath, diag.str());
        bundle.exported_files.push_back(diagPath);
    }

    if (!WriteTextFile(jsonPath, json.str()) || !WriteTextFile(csvPath, csv.str()) || !WriteTextFile(manifestPath, manifest.str()))
    {
        return false;
    }

    bundle.exported_files.push_back(jsonPath);
    bundle.exported_files.push_back(csvPath);
    bundle.exported_files.push_back(manifestPath);
    ++bundle.exported_json_file_count;
    ++bundle.exported_csv_file_count;
    return true;
}

} // namespace rt
