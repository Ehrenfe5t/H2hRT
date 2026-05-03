// 文件目标：
// - 声明模块5批次8的信道统计构建接口。
//
// 主要功能：
// - 根据路径级 EMPathResultSet 生成大尺度/小尺度统计摘要；
// - 为批次8统计域结果闭环提供结构化输出；
// - 为后续模块6报告导出提供统一统计入口。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建信道统计结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化信道统计结果。</returns>
ChannelStatistics BuildChannelStatistics(const EMPathResultSet& pathResults);

} // namespace rt
