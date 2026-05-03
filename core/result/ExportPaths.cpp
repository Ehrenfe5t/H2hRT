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
    if (!EnsureResultDirectory(context.export_root_directory + "/paths"))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n  \"path_count\": " << context.precise_result->path_results.results.size() << ",\n  \"paths\": [\n";
    for (std::size_t i = 0; i < context.precise_result->path_results.results.size(); ++i)
    {
        const EMPathResult& item = context.precise_result->path_results.results[i];
        json << "    { \"path_id\": " << item.path_id
             << ", \"delay_s\": " << item.delay_s
             << ", \"phase_rad\": " << item.phase_rad
             << ", \"power_linear\": " << item.power_linear << " }";
        if (i + 1U < context.precise_result->path_results.results.size())
        {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n}\n";

    std::ostringstream csv;
    csv << "path_id,delay_s,phase_rad,power_linear\n";
    for (const EMPathResult& item : context.precise_result->path_results.results)
    {
        csv << item.path_id << "," << item.delay_s << "," << item.phase_rad << "," << item.power_linear << "\n";
    }

    if (!WriteTextFile(jsonPath, json.str()) || !WriteTextFile(csvPath, csv.str()))
    {
        return false;
    }

    bundle.exported_files.push_back(jsonPath);
    bundle.exported_files.push_back(csvPath);
    return true;
}

} // namespace rt
