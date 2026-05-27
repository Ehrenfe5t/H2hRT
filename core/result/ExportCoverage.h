// 文件目标：
// - 声明模块6批次9的覆盖结果导出接口。
//
// 主要功能：
// - 将 CoverageEM 相关结果导出为结构化文件；
// - 支持人工核查覆盖功率趋势；
// - 为后续 coverage 可视化提供基础数据。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 导出覆盖结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportCoverage(const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
