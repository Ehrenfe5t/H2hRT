// 文件目标：
// - 声明批次A1真实生产链最小贯通入口。
//
// 主要功能：
// - 消费模块4真实 SearchEngineResult；
// - 驱动模块5形成真实路径级 EM 结果集与模块5汇总结果；
// - 驱动模块6形成真实导出、最小 validation 与最小 regression 闭环；
// - 为 RtPipeline 提供 A1 主链切换后的统一执行入口。

#pragma once

#include "../core/common/config/AppConfig.h"
#include "../core/common/log/Logger.h"
#include "../core/em/EMProfile.h"
#include "../core/result/ExportBundle.h"
#include "../core/result/RegressionReport.h"
#include "../core/result/ValidationReport.h"
#include "../core/scene/Scene.h"
#include "../core/search/SearchEngine.h"

namespace rt {

/// <summary>
/// 描述 A1 真实生产链的执行结果。
/// </summary>
struct A1RealChainRunResult {
    bool succeeded = false;
    bool used_reference_path_fallback = false;
    EMPathResultSet path_result_set;
    EMAggregateResult precise_result;
    EMAggregateResult coverage_result;
    ExportBundle export_bundle;
    ValidationReport validation_report;
    RegressionReport regression_report;
};

/// <summary>
/// 执行 A1 真实生产链最小闭环。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="searchResult">模块4真实搜索结果。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>结构化的 A1 主链执行结果。</returns>
A1RealChainRunResult RunA1RealChain(
    const AppConfig& config,
    const Scene& scene,
    const SearchEngineResult& searchResult,
    Logger& logger);

} // namespace rt
