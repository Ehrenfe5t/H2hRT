// 文件目标：
// - 声明模块4批次6的几何合法性检查接口。
//
// 主要功能：
// - 为 LOS、反射、透射、绕射候选提供统一合法性判定结果；
// - 输出结构化失败原因，满足批次6调试与统计需求；
// - 将候选发现与路径成立明确分层。

#pragma once

#include "../path/PathSearchContext.h"

#include <string>

namespace rt {

/// <summary>
/// 几何合法性失败原因枚举。
/// </summary>
enum class GeometryValidityReason {
    Ok = 0,
    InvalidContext,
    NoHit,
    NoCandidate,
    DisabledByFlags,
    InvalidDirection,
    VisibilityBlocked,
    InvalidMediumTransition,
    DegenerateWedge,
    OutOfBudget,
    InvalidState,
    DuplicateInteractionLoop,
    InvalidPathSequence,
    MixedPathNotAllowed,
    PathDepthExceeded,
    CandidateRejectedByControl,
    TotalInternalReflection,     // v9-StageC
    SnellResidualExceeded        // v9-StageC
};

/// <summary>
/// 几何合法性检查结果。
/// </summary>
struct GeometryValidityResult {
    bool valid = false;
    GeometryValidityReason reason = GeometryValidityReason::Ok;
    std::string detail;
};

/// <summary>
/// 检查当前搜索状态是否还允许继续扩展。
/// </summary>
/// <param name="state">待检查状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsSearchStateExpandable(const PathState& state);

/// <summary>
/// 检查 LOS 几何路径是否成立。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidLosPath(const PathSearchContext& context, const PathState& state);

/// <summary>
/// 检查反射候选面元命中是否有效。
/// </summary>
/// <param name="hit">面元命中结果。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidReflectionHitCandidate(const FaceHit& hit, const PathSearchContext& context);

/// <summary>
/// 检查透射候选面元命中是否有效。
/// </summary>
/// <param name="hit">面元命中结果。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidTransmissionHitCandidate(const FaceHit& hit, const PathSearchContext& context);

/// <summary>
/// 检查绕射候选是否有效。
/// </summary>
/// <param name="candidate">楔边候选结果。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidDiffractionCandidate(const WedgeCandidate& candidate);

/// <summary>
/// 检查新生成状态是否满足最小几何约束。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">新状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidExpandedState(const PathSearchContext& context, const PathState& state);

} // namespace rt
