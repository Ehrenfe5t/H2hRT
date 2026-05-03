// 文件目标：
// - 声明模块6批次9的通感结果导出接口。
//
// 主要功能：
// - 将第一版 ISACFeatureSet 导出为结构化文件；
// - 支持人工核查路径数、最早时延与最强路径特征；
// - 为后续通感分析脚本保留数据接口。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 导出通感结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportISAC(const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
