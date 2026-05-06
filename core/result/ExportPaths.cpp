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
                if (candidatePath.path_signature == item.source_path_signature)
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
             << ", \"is_los\": " << (item.is_los ? "true" : "false")
             << ", \"contains_transmission\": " << (item.contains_transmission ? "true" : "false")
             << ", \"transmission_semantic_consumed\": " << (item.transmission_semantic_consumed ? "true" : "false")
             << ", \"first_transmission_medium_in_id\": " << item.first_transmission_medium_in_id
             << ", \"first_transmission_medium_out_id\": " << item.first_transmission_medium_out_id
             << ", \"source_tag\": \"" << item.source_tag << "\""
             << ", \"tx_antenna_id\": \"" << item.tx_antenna_id << "\""
             << ", \"tx_antenna_source_type\": \"" << item.tx_antenna_source_type << "\""
             << ", \"rx_antenna_id\": \"" << item.rx_antenna_id << "\""
             << ", \"rx_antenna_source_type\": \"" << item.rx_antenna_source_type << "\""
             << ", \"source_path_signature\": \"" << item.source_path_signature << "\""
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
                     << " }";
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

    std::ostringstream csv;
    csv << "path_id,delay_s,phase_rad,power_linear,free_space_loss_db,polarization_magnitude,is_los,contains_transmission,transmission_semantic_consumed,first_transmission_medium_in_id,first_transmission_medium_out_id,source_tag,tx_antenna_id,tx_antenna_source_type,rx_antenna_id,rx_antenna_source_type,source_path_signature,node_count\n";
    for (const EMPathResult& item : context.precise_result->path_results.results)
    {
        std::size_t nodeCount = 0;
        if (context.search_result != nullptr)
        {
            for (const GeometricPath& candidatePath : context.search_result->path_set.paths)
            {
                if (candidatePath.path_signature == item.source_path_signature)
                {
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
            << item.source_tag << ","
            << item.tx_antenna_id << ","
            << item.tx_antenna_source_type << ","
            << item.rx_antenna_id << ","
            << item.rx_antenna_source_type << ","
            << item.source_path_signature << "," << nodeCount << "\n";
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
