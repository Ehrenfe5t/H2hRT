// 文件目标：
// - 实现模块6批次9的覆盖结果导出逻辑。
//
// 主要功能：
// - 输出 CoverageEM 汇总 JSON；
// - 提供覆盖功率与贡献路径数摘要；
// - 为模块6覆盖结果表达闭环提供基础文件。

#include "ExportCoverage.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出覆盖结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportCoverage(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/coverage";
    const std::string path = dir + "/coverage_summary.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n"
         << "  \"total_received_power_linear\": " << context.coverage_result->coverage.total_received_power_linear << ",\n"
         << "  \"contributing_path_count\": " << context.coverage_result->coverage.contributing_path_count << "\n"
         << "}\n";

    if (!WriteTextFile(path, json.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    return true;
}

} // namespace rt
