// 文件目标：
// - 实现模块6批次9的验证报告构建与导出逻辑。
//
// 主要功能：
// - 汇总导出文件完整性与关键结果存在性；
// - 输出 ValidationReport JSON；
// - 支持批次9验证闭环检查。

#include "ValidationReportWriter.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 构建验证报告。
/// </summary>
/// <param name="bundle">当前导出总容器。</param>
/// <param name="context">结果导出上下文。</param>
/// <returns>结构化验证报告。</returns>
ValidationReport BuildValidationReport(const ExportBundle& bundle, const ResultExportContext& context)
{
    ValidationReport report;
    ValidationItem fileItem;
    fileItem.name = "exported_file_count";
    fileItem.passed = !bundle.exported_files.empty();
    fileItem.detail = std::to_string(bundle.exported_files.size());
    report.items.push_back(fileItem);

    ValidationItem preciseItem;
    preciseItem.name = "precise_result_exists";
    preciseItem.passed = context.precise_result != nullptr && !context.precise_result->path_results.results.empty();
    preciseItem.detail = preciseItem.passed ? "true" : "false";
    report.items.push_back(preciseItem);

    ValidationItem coverageItem;
    coverageItem.name = "coverage_result_exists";
    coverageItem.passed = context.coverage_result != nullptr && context.coverage_result->coverage.contributing_path_count >= 0;
    coverageItem.detail = coverageItem.passed ? "true" : "false";
    report.items.push_back(coverageItem);

    report.passed = fileItem.passed && preciseItem.passed && coverageItem.passed;
    return report;
}

/// <summary>
/// 导出验证报告文件。
/// </summary>
/// <param name="report">验证报告对象。</param>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportValidationReport(const ValidationReport& report, const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/reports";
    const std::string path = dir + "/validation_report.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }
    std::ostringstream json;
    json << "{\n  \"passed\": " << (report.passed ? "true" : "false") << ",\n  \"items\": [\n";
    for (std::size_t i = 0; i < report.items.size(); ++i)
    {
        const ValidationItem& item = report.items[i];
        json << "    { \"name\": \"" << item.name << "\", \"passed\": " << (item.passed ? "true" : "false") << ", \"detail\": \"" << item.detail << "\" }";
        if (i + 1U < report.items.size()) json << ",";
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
