// 文件目标：
// - 声明模块6批次9的可视化辅助导出接口。
//
// 主要功能：
// - 输出路径线与命中点的第一版辅助 JSON；
// - 为后续可视化脚本直接消费提供基础文件；
// - 满足主文档对 visualization_export 的最小闭环要求。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 导出可视化辅助文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportVisualization(const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
