// 文件目标：
// - 声明模块4批次6的绕射扩展器接口。
//
// 主要功能：
// - 基于当前状态查询楔边候选；
// - 对候选做最小几何合法性检查；
// - 生成新的绕射 PathState。

#pragma once

#include "ReflectionExpander.h"

namespace rt {

/// <summary>
/// 执行一次绕射扩展。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化绕射扩展结果。</returns>
ExpanderResult ExpandDiffraction(const PathSearchContext& context, const PathState& state);

} // namespace rt
