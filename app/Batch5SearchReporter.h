// 文件目标：
// - 声明批次5 SearchEngine 骨架验证输出函数。
//
// 主要功能：
// - 输出状态数、去重数、路径数与关键 trace；
// - 满足主文档批次5检测方式中的日志验证需求；
// - 为后续批次6扩展器接入提供统一调试出口。

#pragma once

#include "../core/common/log/Logger.h"
#include "../core/search/SearchEngine.h"

namespace rt {

/// <summary>
/// 将批次5 SearchEngine 摘要输出到日志系统。
/// </summary>
/// <param name="result">搜索执行结果。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportBatch5SearchSummary(const SearchEngineResult& result, Logger& logger);

} // namespace rt
