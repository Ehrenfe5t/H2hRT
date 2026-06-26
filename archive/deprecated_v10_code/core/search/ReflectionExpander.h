// v11: DEPRECATED — 模块4批次6的反射扩展器接口.
// v11 P2P 主链已切换到 SbrEngine::RunPointToPoint(), 不再使用 SearchEngine + Expander.
// 保留用于历史对照.
//
// 主要功能：
// - 基于当前状态与场景查询发现反射候选；
// - 构造新的反射 PathState；
// - 输出结构化失败原因，便于批次6统计与调试。

#pragma once

#include "GeometryValidity.h"

#include <vector>

namespace rt {

/// <summary>
/// 扩展器执行结果。
/// </summary>
struct ExpanderResult {
    std::vector<PathState> next_states;
    std::vector<GeometryValidityReason> failure_reasons;
};

/// <summary>
/// 执行一次反射扩展。
/// </summary>
ExpanderResult ExpandReflection(const PathSearchContext& context, const PathState& state);

/// <summary>v8: 约束候选面元集 (PVS筛选), nullptr=全场景</summary>
ExpanderResult ExpandReflection(const PathSearchContext& context, const PathState& state,
                                const std::vector<int>* candidateFaces);

} // namespace rt
