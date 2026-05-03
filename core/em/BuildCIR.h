// 文件目标：
// - 声明模块5批次8的 CIR 构建接口。
//
// 主要功能：
// - 根据路径级 EMPathResultSet 构建 CIRResult；
// - 支持 PreciseEM / CoverageEM profile 区分；
// - 为后续 PDP 与时延域分析提供统一输入。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建 CIR 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <param name="profile">当前求值 profile。</param>
/// <returns>结构化 CIR 结果。</returns>
CIRResult BuildCIR(const EMPathResultSet& pathResults, const EMSolveProfile& profile);

} // namespace rt
