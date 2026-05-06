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

namespace {

std::string InferValidationModule(const std::string& itemName)
{
    if (itemName.find("antenna") != std::string::npos)
    {
        return "Module3/Module5";
    }
    if (itemName.find("transmission") != std::string::npos)
    {
        return "Module5";
    }
    if (itemName.find("search") != std::string::npos)
    {
        return "Module4";
    }
    if (itemName.find("coverage") != std::string::npos || itemName.find("aggregate") != std::string::npos)
    {
        return "Module5/Module6";
    }
    if (itemName.find("file") != std::string::npos)
    {
        return "Module6";
    }
    return "Module6";
}

} // namespace

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

    ValidationItem searchItem;
    searchItem.name = "search_result_exists";
    searchItem.passed = !context.real_chain_enabled || (context.search_result != nullptr && !context.search_result->path_set.paths.empty());
    searchItem.detail = context.real_chain_enabled
        ? (searchItem.passed ? std::to_string(context.search_result->path_set.paths.size()) : "0")
        : "legacy_batch9_self_check";
    report.items.push_back(searchItem);

    ValidationItem realChainItem;
    realChainItem.name = "real_chain_enabled";
    realChainItem.passed = context.real_chain_enabled || context.primary_input_source.empty();
    realChainItem.detail = context.real_chain_enabled ? context.primary_input_source : "legacy_batch9_self_check";
    report.items.push_back(realChainItem);

    ValidationItem noReferenceFallbackItem;
    noReferenceFallbackItem.name = "no_reference_path_primary";
    noReferenceFallbackItem.passed = !context.real_chain_enabled || !bundle.used_reference_path_fallback;
    noReferenceFallbackItem.detail = context.real_chain_enabled
        ? (noReferenceFallbackItem.passed ? "true" : "false")
        : "legacy_batch9_self_check";
    report.items.push_back(noReferenceFallbackItem);

    ValidationItem coverageItem;
    coverageItem.name = "coverage_result_exists";
    coverageItem.passed = context.coverage_result != nullptr && context.coverage_result->coverage.contributing_path_count >= 0;
    coverageItem.detail = coverageItem.passed ? "true" : "false";
    report.items.push_back(coverageItem);

    ValidationItem antennaInputItem;
    antennaInputItem.name = "formal_antenna_input_visible";
    antennaInputItem.passed = true;
    if (context.precise_result != nullptr && !context.precise_result->path_results.results.empty())
    {
        const EMPathResult& firstResult = context.precise_result->path_results.results.front();
        antennaInputItem.passed = !firstResult.tx_antenna_id.empty() && !firstResult.tx_antenna_source_type.empty();
        antennaInputItem.detail = firstResult.tx_antenna_id + "/" + firstResult.tx_antenna_source_type;
    }
    else
    {
        antennaInputItem.detail = "no_precise_result";
        antennaInputItem.passed = false;
    }
    report.items.push_back(antennaInputItem);

    ValidationItem aggregateTrendItem;
    aggregateTrendItem.name = "aggregate_trace_quality";
    aggregateTrendItem.passed = context.precise_result != nullptr &&
                                context.precise_result->statistics.valid_path_count > 0 &&
                                context.precise_result->coverage.average_free_space_loss_db >= 0.0;
    if (context.precise_result != nullptr)
    {
        std::ostringstream detail;
        detail << "paths=" << context.precise_result->statistics.valid_path_count
               << ", mean_abs_phase=" << context.precise_result->statistics.mean_abs_phase_rad
               << ", avg_fs_loss_db=" << context.precise_result->coverage.average_free_space_loss_db;
        aggregateTrendItem.detail = detail.str();
    }
    else
    {
        aggregateTrendItem.detail = "no_precise_result";
    }
    report.items.push_back(aggregateTrendItem);

    int transmissionPathCount = 0;
    int transmissionConsumedCount = 0;
    if (context.precise_result != nullptr)
    {
        for (const EMPathResult& item : context.precise_result->path_results.results)
        {
            if (item.contains_transmission)
            {
                ++transmissionPathCount;
            }
            if (item.transmission_semantic_consumed)
            {
                ++transmissionConsumedCount;
            }
        }
    }

    ValidationItem transmissionTraceItem;
    transmissionTraceItem.name = "transmission_trace_exists";
    transmissionTraceItem.passed = (transmissionPathCount == 0) || (transmissionConsumedCount > 0);
    transmissionTraceItem.detail = std::to_string(transmissionPathCount) + "/" + std::to_string(transmissionConsumedCount);
    report.items.push_back(transmissionTraceItem);

    int transmissionMediumMismatchCount = 0;
    if (context.precise_result != nullptr)
    {
        for (const EMPathResult& item : context.precise_result->path_results.results)
        {
            if (!item.contains_transmission)
            {
                continue;
            }
            if (item.first_transmission_medium_in_id < 0 ||
                item.first_transmission_medium_out_id < 0 ||
                item.first_transmission_medium_in_id == item.first_transmission_medium_out_id)
            {
                ++transmissionMediumMismatchCount;
            }
        }
    }

    ValidationItem transmissionFallbackItem;
    transmissionFallbackItem.name = "transmission_no_obvious_fallback";
    transmissionFallbackItem.passed = transmissionMediumMismatchCount == 0;
    transmissionFallbackItem.detail = std::to_string(transmissionMediumMismatchCount);
    report.items.push_back(transmissionFallbackItem);

    for (const ValidationItem& item : report.items)
    {
        if (item.passed)
        {
            ++report.passed_item_count;
        }
        else
        {
            ++report.failed_item_count;
        }
    }

    report.passed = fileItem.passed && preciseItem.passed && searchItem.passed && realChainItem.passed && noReferenceFallbackItem.passed && coverageItem.passed && transmissionTraceItem.passed && transmissionFallbackItem.passed && antennaInputItem.passed && aggregateTrendItem.passed;
    report.summary_level = report.passed ? "info" : "error";
    if (!report.passed)
    {
        for (const ValidationItem& item : report.items)
        {
            if (!item.passed)
            {
                report.failed_module = InferValidationModule(item.name);
                report.failed_reason = item.name;
                report.failed_sample = context.config != nullptr ? context.config->experiment.dataset_tag : "unknown_sample";
                break;
            }
        }
    }
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
    json << "{\n  \"export_schema_version\": \"" << bundle.export_schema_version << "\""
         << ",\n  \"export_purpose\": \"" << context.export_purpose << "\""
         << ",\n  \"passed\": " << (report.passed ? "true" : "false")
         << ",\n  \"passed_item_count\": " << report.passed_item_count
         << ",\n  \"failed_item_count\": " << report.failed_item_count
         << ",\n  \"summary_level\": \"" << report.summary_level << "\""
         << ",\n  \"failed_module\": \"" << report.failed_module << "\""
         << ",\n  \"failed_reason\": \"" << report.failed_reason << "\""
         << ",\n  \"failed_sample\": \"" << report.failed_sample << "\""
         << ",\n  \"items\": [\n";
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
    ++bundle.exported_json_file_count;
    return true;
}

} // namespace rt
