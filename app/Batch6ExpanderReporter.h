// 文件目标：
// - 声明批次6扩展器验证输出函数。
//
// 主要功能：
// - 对 Reflection / Transmission / Diffraction 扩展器执行批次级自检；
// - 输出扩展器成功/失败统计与摘要；
// - 为批次6闭环提供统一日志出口。

#pragma once

#include "../core/common/log/Logger.h"
#include "../core/path/PathSearchContext.h"

namespace rt {

/// <summary>
/// 执行批次6扩展器自检并输出摘要。
/// </summary>
/// <param name="context">批次6调试搜索上下文。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次6自检通过；false 表示失败。</returns>
bool ReportBatch6ExpanderSummary(const PathSearchContext& context, Logger& logger);

} // namespace rt
