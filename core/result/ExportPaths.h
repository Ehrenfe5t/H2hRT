// 文件目标：
// - 声明模块6批次9的路径结果导出接口。
//
// 主要功能：
// - 将路径级 EM 结果导出为 JSON / CSV；
// - 为人工核查和后续可视化提供路径级数据文件；
// - 作为模块6结果表达层的重要组成部分。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 导出路径级结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportPaths(const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
