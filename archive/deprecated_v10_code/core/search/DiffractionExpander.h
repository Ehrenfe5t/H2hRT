// v11: DEPRECATED — 模块4批次6的绕射扩展器接口.
// v11 P2P 主链已切换到 SbrEngine::RunPointToPoint(), 不再使用 SearchEngine + Expander.
// 保留用于历史对照.
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
ExpanderResult ExpandDiffraction(const PathSearchContext& context, const PathState& state);

/// <summary>v8: 约束候选楔边集, nullptr=全场景</summary>
ExpanderResult ExpandDiffraction(const PathSearchContext& context, const PathState& state,
                                 const std::vector<int>* candidateWedges);

} // namespace rt
