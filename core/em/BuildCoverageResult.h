// 文件目标：
// - 声明模块5批次8的覆盖结果构建接口。
//
// 主要功能：
// - 根据路径级 EMPathResultSet 构建 CoverageResult；
// - 支持 CoverageEM 的非相干功率汇总；
// - 为后续大规模 coverage 扩展保留统一入口。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建覆盖结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <param name="profile">当前求值 profile。</param>
/// <returns>结构化覆盖结果。</returns>
CoverageResult BuildCoverageResult(const EMPathResultSet& pathResults, const EMSolveProfile& profile);

} // namespace rt
