// 文件目标：
// - 定义模块6批次9的回归报告结构。
//
// 主要功能：
// - 汇总当前结果与参考摘要之间的差异；
// - 提供可人工审阅的回归结论；
// - 为后续版本比较和 regression 工具提供统一对象。

#pragma once

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 回归差异条目。
/// </summary>
struct RegressionDiffItem {
    std::string metric_name;
    std::string current_value;
    std::string baseline_value;
    std::string severity = "info";
    std::string reason;
};

/// <summary>
/// 回归报告结构。
/// </summary>
struct RegressionReport {
    std::vector<RegressionDiffItem> diff_items;
    std::string baseline_name;
    std::string current_run_name;
    int diff_item_count = 0;
    int warning_count = 0;
    int blocking_count = 0;
    std::string blocking_module = "";
    std::string blocking_reason = "";
    std::string blocking_sample = "";
    bool has_blocking_diff = false;
};

} // namespace rt
