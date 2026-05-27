// 文件目标：
// - 声明模块6批次9的信道结果导出接口。
//
// 主要功能：
// - 将 CIR / PDP / APS / Statistics 导出为结构化文件；
// - 为后续可视化与论文后处理提供基础文件；
// - 完成模块6信道结果表达闭环的一部分。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 导出信道结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportChannel(const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
