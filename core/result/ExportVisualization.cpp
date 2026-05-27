// 文件目标：
// - 实现模块6批次9的可视化辅助导出逻辑。
//
// 主要功能：
// - 输出路径线和命中点的基础 JSON；
// - 供后续可视化脚本直接加载；
// - 为模块6可视化辅助文件闭环提供基础格式。

#include "ExportVisualization.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出可视化辅助文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportVisualization(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/visualization";
    const std::string path = dir + "/path_points.json";
    const std::string manifestPath = dir + "/inspection_manifest.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n  \"export_schema_version\": \"" << bundle.export_schema_version << "\",\n"
         << "  \"export_purpose\": \"" << context.export_purpose << "\",\n"
         << "  \"handoff_view_name\": \"" << context.handoff_view_name << "\",\n"
         << "  \"paths\": [\n";
    for (std::size_t i = 0; i < context.precise_result->path_results.results.size(); ++i)
    {
        const EMPathResult& item = context.precise_result->path_results.results[i];
        json << "    { \"path_id\": " << item.path_id << ", \"power_linear\": " << item.power_linear << ", \"phase_rad\": " << item.phase_rad << ", \"free_space_loss_db\": " << item.free_space_loss_db << ", \"contains_transmission\": " << (item.contains_transmission ? "true" : "false") << ", \"source_tag\": \"" << item.source_tag << "\", \"tx_antenna_source_type\": \"" << item.tx_antenna_source_type << "\", \"rx_antenna_source_type\": \"" << item.rx_antenna_source_type << "\" }";
        if (i + 1U < context.precise_result->path_results.results.size())
        {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n}\n";

    std::ostringstream manifest;
    manifest << "{\n"
             << "  \"export_schema_version\": \"" << bundle.export_schema_version << "\",\n"
             << "  \"handoff_view_name\": \"" << context.handoff_view_name << "\",\n"
             << "  \"export_purpose\": \"" << context.export_purpose << "\",\n"
             << "  \"path_points_file\": \"visualization/path_points.json\",\n"
             << "  \"inspection_scope\": \"path_points_and_path_trace\",\n"
             << "  \"path_count\": " << context.precise_result->path_results.results.size() << "\n"
             << "}\n";

    if (!WriteTextFile(path, json.str()) || !WriteTextFile(manifestPath, manifest.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    bundle.exported_files.push_back(manifestPath);
    ++bundle.exported_json_file_count;
    return true;
}

} // namespace rt
