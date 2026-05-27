// 文件目标：
// - 声明模块4批次6的透射介质切换解析接口。
//
// 主要功能：
// - 根据 FaceHit 与当前介质状态解析 medium in/out；
// - 为 TransmissionExpander 提供显式介质切换结果；
// - 避免透射链路发生关键静默 fallback。

#pragma once

#include "../path/PathSearchContext.h"

namespace rt {

/// <summary>
/// 透射介质切换结果。
/// </summary>
struct MediumTransitionInfo {
    bool valid = false;
    int medium_in_id = -1;
    int medium_out_id = -1;
    int front_medium_id = -1;
    int back_medium_id = -1;
    int front_material_id = -1;
    int back_material_id = -1;
    int face_id = -1;
    int object_id = -1;
    bool entered_from_front_side = true;
    bool dual_side_semantic_complete = false;
};

/// <summary>
/// 解析透射交互下的介质切换信息。
/// </summary>
/// <param name="state">当前路径状态。</param>
/// <param name="hit">透射候选面元命中。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化介质切换结果。</returns>
MediumTransitionInfo ResolveMediumTransition(const PathState& state, const FaceHit& hit, const PathSearchContext& context);

} // namespace rt
