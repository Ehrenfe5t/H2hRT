// 文件目标：
// - 声明模块4批次6的反射扩展器接口。
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
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化反射扩展结果。</returns>
ExpanderResult ExpandReflection(const PathSearchContext& context, const PathState& state);

} // namespace rt
