// 文件目标：
// - 声明模块5批次8的 PDP 构建接口。
//
// 主要功能：
// - 根据 CIR 或路径级结果生成 PDPResult；
// - 保留时延与功率摘要；
// - 为批次8时延域分析提供统一输出。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建 PDP 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化 PDP 结果。</returns>
PDPResult BuildPDP(const EMPathResultSet& pathResults);

} // namespace rt
