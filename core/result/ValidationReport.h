// 文件目标：
// - 定义模块6批次9的验证报告结构。
//
// 主要功能：
// - 汇总当前运行中的关键验证项与通过状态；
// - 为人工核查和后续自动对照提供稳定报告对象；
// - 作为结果表达层的正式验证承载结构。

#pragma once

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 单项验证记录。
/// </summary>
struct ValidationItem {
    std::string name;
    bool passed = false;
    std::string detail;
};

/// <summary>
/// 验证报告结构。
/// </summary>
struct ValidationReport {
    std::vector<ValidationItem> items;
    bool passed = false;
};

} // namespace rt
