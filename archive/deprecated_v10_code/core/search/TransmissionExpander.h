// v11: DEPRECATED — 模块4批次6的透射扩展器接口.
// v11 P2P 主链已切换到 SbrEngine::RunPointToPoint(), 不再使用 SearchEngine + Expander.
// 保留用于历史对照.
//
// 主要功能：
// - 基于当前状态与场景查询发现透射候选；
// - 调用介质切换解析逻辑；
// - 生成新的透射 PathState。

#pragma once

#include "ReflectionExpander.h"

namespace rt {

/// <summary>
/// 执行一次透射扩展。
/// </summary>
ExpanderResult ExpandTransmission(const PathSearchContext& context, const PathState& state);

/// <summary>v8: 约束候选面元集, nullptr=全场景</summary>
ExpanderResult ExpandTransmission(const PathSearchContext& context, const PathState& state,
                                  const std::vector<int>* candidateFaces);

} // namespace rt
