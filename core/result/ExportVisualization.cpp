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
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n  \"paths\": [\n";
    for (std::size_t i = 0; i < context.precise_result->path_results.results.size(); ++i)
    {
        const EMPathResult& item = context.precise_result->path_results.results[i];
        json << "    { \"path_id\": " << item.path_id << ", \"power_linear\": " << item.power_linear << " }";
        if (i + 1U < context.precise_result->path_results.results.size())
        {
            json << ",";
        }
        json << "\n";
    }
    json << "  ]\n}\n";

    if (!WriteTextFile(path, json.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    return true;
}

} // namespace rt
