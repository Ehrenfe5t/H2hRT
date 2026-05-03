// 文件目标：
// - 声明模块5批次8的 PreciseEM profile 构建接口。
//
// 主要功能：
// - 生成面向高保真路径级结果保留的 profile；
// - 为批次8 precise / coverage 对比提供稳定入口；
// - 保持 profile 定义与求值主链解耦。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建 PreciseEM profile。
/// </summary>
/// <returns>结构化 PreciseEM profile。</returns>
EMSolveProfile BuildPreciseEMProfile();

} // namespace rt
