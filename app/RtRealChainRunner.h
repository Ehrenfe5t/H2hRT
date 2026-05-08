// ───────────────────────────────────────────────────────────────────
// 文件: RtRealChainRunner.h
// 用途: RT真实生产链运行器声明。消费SearchEngine的真实路径集，驱动逐路径
//       EM求解(模块5)，产出聚合结果(Precise+Coverage双profile)，执行模块6
//       导出/验证/回归闭环。
// 所属模块: 应用层
// ───────────────────────────────────────────────────────────────────

#pragma once

#include "../core/common/config/AppConfig.h"
#include "../core/common/log/Logger.h"
#include "../core/common/material/MaterialDatabase.h"
#include "../core/em/EMProfile.h"
#include "../core/result/ExportBundle.h"
#include "../core/result/RegressionReport.h"
#include "../core/result/ValidationReport.h"
#include "../core/scene/Scene.h"
#include "../core/search/SearchEngine.h"

namespace rt {

/// <summary>
/// 单次 A1 真实生产链运行的完整结果。
/// </summary>
struct A1RealChainRunResult {
    bool succeeded = false;
    bool used_reference_path_fallback = false;
    EMPathResultSet path_result_set;       // 每条真实搜索路径的EM结果
    EMAggregateResult precise_result;      // Precise-profile聚合 (CIR, PDP, APS, 统计)
    EMAggregateResult coverage_result;     // Coverage-profile聚合
    ExportBundle export_bundle;             // 全部导出文件摘要
    ValidationReport validation_report;     // 模块6验证结论
    RegressionReport regression_report;     // 模块6回归结论
};

/// <summary>
/// 执行 A1 真实生产链: 逐路径EM求解 → 聚合profile → 导出 → 验证 → 回归闭环。
/// </summary>
/// <param name="config">应用配置。</param>
/// <param name="scene">含闭环几何与查询门面的静态场景。</param>
/// <param name="searchResult">SearchEngine 的真实搜索结果 (模块4输出)。</param>
/// <param name="logger">统一日志器。</param>
/// <param name="materialDb">可选材质数据库, 供介电常数查询。</param>
/// <returns>包含全部EM聚合和导出产物的结构化A1链结果。</returns>
A1RealChainRunResult RunA1RealChain(
    const AppConfig& config,
    const Scene& scene,
    const SearchEngineResult& searchResult,
    Logger& logger,
    const MaterialDatabase* materialDb = nullptr);

} // namespace rt
