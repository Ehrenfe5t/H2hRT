// 文件目标：
// - 声明模块6批次9的回归报告构建与导出接口。
//
// 主要功能：
// - 基于 PreciseEM / CoverageEM 汇总结果构建差异摘要；
// - 将 RegressionReport 导出为 JSON 文件；
// - 为后续 regression 工具保留统一入口。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 构建回归报告。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <returns>结构化回归报告。</returns>
RegressionReport BuildRegressionReport(const ResultExportContext& context);

/// <summary>
/// 导出回归报告文件。
/// </summary>
/// <param name="report">回归报告对象。</param>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportRegressionReport(const RegressionReport& report, const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
