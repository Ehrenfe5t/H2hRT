// 文件目标：
// - 声明批次9结果表达、验证与回归闭环验证输出函数。
//
// 主要功能：
// - 组织 ExportBundle、ValidationReport、RegressionReport 的生成；
// - 输出导出文件数与报告摘要；
// - 为批次9闭环提供统一日志出口。

#pragma once

#include "../core/common/log/Logger.h"
#include "../core/common/config/AppConfig.h"
#include "../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 执行批次9结果表达、验证与回归自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次9自检通过；false 表示失败。</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger);

} // namespace rt
