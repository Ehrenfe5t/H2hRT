// 文件目标：
// - 声明模块5批次8的 CoverageEM profile 构建接口。
//
// 主要功能：
// - 生成面向覆盖仿真的简化 profile；
// - 为批次8 precise / coverage 对比提供统一入口；
// - 保持 CoverageEM 简化边界显式化。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建 CoverageEM profile。
/// </summary>
/// <returns>结构化 CoverageEM profile。</returns>
EMSolveProfile BuildCoverageEMProfile();

} // namespace rt
