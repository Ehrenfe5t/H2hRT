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

namespace {

std::string InferRegressionModule(const std::string& metricName)
{
    if (metricName.find("transmission") != std::string::npos)
    {
        return "Module5";
    }
    if (metricName.find("path") != std::string::npos)
    {
        return "Module4/Module5";
    }
    if (metricName.find("source") != std::string::npos)
    {
        return "Module6";
    }
    if (metricName.find("phase") != std::string::npos || metricName.find("power") != std::string::npos || metricName.find("delay") != std::string::npos)
    {
        return "Module5";
    }
    return "Module6";
}

} // namespace

/// <summary>
/// 构建回归报告。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <returns>结构化回归报告。</returns>
RegressionReport BuildRegressionReport(const ResultExportContext& context)
{
    RegressionReport report;
    report.baseline_name = "coverage_profile_baseline";
    report.current_run_name = context.primary_input_source.empty() ? "legacy_current" : context.primary_input_source;

    RegressionDiffItem item;
    item.metric_name = "total_power_linear";
    item.current_value = std::to_string(context.precise_result->statistics.total_power_linear);
    item.baseline_value = std::to_string(context.coverage_result->statistics.total_power_linear);
    item.severity = "info";
    item.reason = "profile_level_difference_observation";
    report.diff_items.push_back(item);

    RegressionDiffItem pathCountItem;
    pathCountItem.metric_name = "valid_path_count";
    pathCountItem.current_value = std::to_string(context.precise_result->statistics.valid_path_count);
    pathCountItem.baseline_value = std::to_string(context.coverage_result->statistics.valid_path_count);
    pathCountItem.severity = "info";
    pathCountItem.reason = "path_count_tracking";
    report.diff_items.push_back(pathCountItem);

    RegressionDiffItem delayItem;
    delayItem.metric_name = "mean_delay_s";
    delayItem.current_value = std::to_string(context.precise_result->statistics.mean_delay_s);
    delayItem.baseline_value = std::to_string(context.coverage_result->statistics.mean_delay_s);
    delayItem.severity = "info";
    delayItem.reason = "delay_tracking";
    report.diff_items.push_back(delayItem);

    RegressionDiffItem phaseItem;
    phaseItem.metric_name = "mean_abs_phase_rad";
    phaseItem.current_value = std::to_string(context.precise_result->statistics.mean_abs_phase_rad);
    phaseItem.baseline_value = std::to_string(context.coverage_result->statistics.mean_abs_phase_rad);
    phaseItem.severity = "info";
    phaseItem.reason = "phase_tracking";
    report.diff_items.push_back(phaseItem);

    RegressionDiffItem sourceItem;
    sourceItem.metric_name = "primary_input_source";
    sourceItem.current_value = context.primary_input_source;
    sourceItem.baseline_value = context.real_chain_enabled ? "real_chain_required" : "unknown";
    sourceItem.severity = (context.real_chain_enabled && context.primary_input_source.empty()) ? "blocking" : "info";
    sourceItem.reason = "real_chain_source_required";
    report.diff_items.push_back(sourceItem);

    int transmissionPathCount = 0;
    int transmissionConsumedCount = 0;
    int transmissionFallbackRiskCount = 0;
    if (context.precise_result != nullptr)
    {
        for (const EMPathResult& item : context.precise_result->path_results.results)
        {
            if (item.contains_transmission)
            {
                ++transmissionPathCount;
                if (item.first_transmission_medium_in_id < 0 ||
                    item.first_transmission_medium_out_id < 0 ||
                    item.first_transmission_medium_in_id == item.first_transmission_medium_out_id)
                {
                    ++transmissionFallbackRiskCount;
                }
            }

            if (item.transmission_semantic_consumed)
            {
                ++transmissionConsumedCount;
            }
        }
    }

    RegressionDiffItem transmissionTraceItem;
    transmissionTraceItem.metric_name = "transmission_semantic_consumed_count";
    transmissionTraceItem.current_value = std::to_string(transmissionConsumedCount);
    transmissionTraceItem.baseline_value = "nonzero_required_for_transmission_paths";
    transmissionTraceItem.severity = (transmissionPathCount > 0 && transmissionConsumedCount == 0) ? "blocking" : "info";
    transmissionTraceItem.reason = "transmission_semantic_must_be_consumed";
    report.diff_items.push_back(transmissionTraceItem);

    RegressionDiffItem transmissionFallbackItem;
    transmissionFallbackItem.metric_name = "transmission_fallback_risk_count";
    transmissionFallbackItem.current_value = std::to_string(transmissionFallbackRiskCount);
    transmissionFallbackItem.baseline_value = transmissionPathCount > 0 ? "0_required" : "n/a";
    transmissionFallbackItem.severity = transmissionFallbackRiskCount > 0 ? "blocking" : "info";
    transmissionFallbackItem.reason = "transmission_should_not_fallback_to_invalid_medium_transition";
    report.diff_items.push_back(transmissionFallbackItem);

    report.diff_item_count = static_cast<int>(report.diff_items.size());
    for (const RegressionDiffItem& diffItem : report.diff_items)
    {
        if (diffItem.severity == "blocking")
        {
            ++report.blocking_count;
            if (report.blocking_module.empty())
            {
                report.blocking_module = InferRegressionModule(diffItem.metric_name);
                report.blocking_reason = diffItem.reason;
                report.blocking_sample = context.config != nullptr ? context.config->experiment.dataset_tag : "unknown_sample";
            }
        }
        else if (diffItem.severity == "warning")
        {
            ++report.warning_count;
        }
    }
    report.has_blocking_diff = report.blocking_count > 0;
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
    json << "{\n  \"export_schema_version\": \"" << bundle.export_schema_version << "\""
         << ",\n  \"export_purpose\": \"" << context.export_purpose << "\""
         << ",\n  \"baseline_name\": \"" << report.baseline_name << "\""
         << ",\n  \"current_run_name\": \"" << report.current_run_name << "\""
         << ",\n  \"diff_item_count\": " << report.diff_item_count
         << ",\n  \"warning_count\": " << report.warning_count
         << ",\n  \"blocking_count\": " << report.blocking_count
         << ",\n  \"blocking_module\": \"" << report.blocking_module << "\""
         << ",\n  \"blocking_reason\": \"" << report.blocking_reason << "\""
         << ",\n  \"blocking_sample\": \"" << report.blocking_sample << "\""
         << ",\n  \"has_blocking_diff\": " << (report.has_blocking_diff ? "true" : "false")
         << ",\n  \"diff_items\": [\n";
    for (std::size_t i = 0; i < report.diff_items.size(); ++i)
    {
        const RegressionDiffItem& item = report.diff_items[i];
        json << "    { \"metric_name\": \"" << item.metric_name
             << "\", \"current_value\": \"" << item.current_value
             << "\", \"baseline_value\": \"" << item.baseline_value
             << "\", \"severity\": \"" << item.severity
             << "\", \"reason\": \"" << item.reason << "\" }";
        if (i + 1U < report.diff_items.size()) json << ",";
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
