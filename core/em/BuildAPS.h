// 文件目标：
// - 声明模块5批次8的 APS 构建接口。
//
// 主要功能：
// - 根据路径级 EMPathResultSet 生成 APSResult；
// - 以基础角度指标占位支持批次8闭环；
// - 为后续更严格 AoA/AoD 角域统计预留接口。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建 APS 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化 APS 结果。</returns>
APSResult BuildAPS(const EMPathResultSet& pathResults);

} // namespace rt
