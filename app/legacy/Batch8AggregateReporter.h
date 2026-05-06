// 文件目标：
// - 声明批次8多路径汇总与双模式求值验证输出函数。
//
// 主要功能：
// - 对 PreciseEM / CoverageEM 两种 profile 执行汇总自检；
// - 输出 CIR / PDP / APS / Statistics / Coverage / ISACFeatureSet 摘要；
// - 为批次8闭环提供统一日志出口。

#pragma once

#include "../../core/common/log/Logger.h"
#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 执行批次8多路径汇总与双模式自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次8自检通过；false 表示失败。</returns>
bool ReportBatch8AggregateSummary(const AppConfig& config, const Scene& scene, Logger& logger);

} // namespace rt
