// ───────────────────────────────────────────────────────────────────
// 文件: Batch9ExportReporter.h
// 用途: Batch-9导出/验证/回归报告器声明。
//       注意: 过渡/遗留模块。Batch-9自检链与A1真实生产链并存，用于交叉验证；
//       它使用手工构造的参考路径而非SearchEngine的真实输出。
// 所属模块: 应用层 (遗留资产)
// ───────────────────────────────────────────────────────────────────

#pragma once

#include "../core/common/log/Logger.h"
#include "../core/common/config/AppConfig.h"
#include "../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 遗留 Batch-9 导出/验证/回归自检。
/// 构建手工参考路径, 运行EM, 导出全部结果类型, 输出摘要日志。
/// 保留用于与 A1 真实链交叉验证。
/// </summary>
/// <param name="config">应用配置。</param>
/// <param name="scene">含闭环几何的静态场景。</param>
/// <param name="logger">统一日志器。</param>
/// <returns>Batch-9自检通过返回true; 否则返回false。</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger);

} // namespace rt
