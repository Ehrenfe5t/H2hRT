// 文件目标：
// - 实现模块6批次9的回归报告构建与导出逻辑。
//
// 主要功能：
// - 比较 PreciseEM 与 CoverageEM 的关键摘要；
// - 生成 RegressionReport JSON；
// - 为批次9回归检查提供基础差异报告。

#include "RegressionReportWriter.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 构建回归报告。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <returns>结构化回归报告。</returns>
RegressionReport BuildRegressionReport(const ResultExportContext& context)
{
    RegressionReport report;
    RegressionDiffItem item;
    item.metric_name = "total_power_linear";
    item.current_value = std::to_string(context.precise_result->statistics.total_power_linear);
    item.baseline_value = std::to_string(context.coverage_result->statistics.total_power_linear);
    report.diff_items.push_back(item);
    report.has_blocking_diff = false;
    return report;
}

/// <summary>
/// 导出回归报告文件。
/// </summary>
/// <param name="report">回归报告对象。</param>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportRegressionReport(const RegressionReport& report, const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/reports";
    const std::string path = dir + "/regression_report.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }
    std::ostringstream json;
    json << "{\n  \"has_blocking_diff\": " << (report.has_blocking_diff ? "true" : "false") << ",\n  \"diff_items\": [\n";
    for (std::size_t i = 0; i < report.diff_items.size(); ++i)
    {
        const RegressionDiffItem& item = report.diff_items[i];
        json << "    { \"metric_name\": \"" << item.metric_name << "\", \"current_value\": \"" << item.current_value << "\", \"baseline_value\": \"" << item.baseline_value << "\" }";
        if (i + 1U < report.diff_items.size()) json << ",";
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
