// 文件目标：
// - 实现模块4批次6的几何合法性检查逻辑。
//
// 主要功能：
// - 为 LOS、反射、透射、绕射扩展输出统一判定结果；
// - 在批次6范围内优先保证失败原因显式可追溯；
// - 为后续剪枝与统计提供结构化失败原因源。

#include "GeometryValidity.h"

#include <cmath>

namespace rt {

namespace {

double Length(const Vec3& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

} // namespace

/// <summary>
/// 检查当前搜索状态是否还允许继续扩展。
/// </summary>
/// <param name="state">待检查状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsSearchStateExpandable(const PathState& state)
{
    GeometryValidityResult result;
    if (!state.valid)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "PathState is marked invalid.";
        return result;
    }
    if (state.remaining_total_expansions <= 0)
    {
        result.reason = GeometryValidityReason::OutOfBudget;
        result.detail = "remaining_total_expansions <= 0.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

/// <summary>
/// 检查 LOS 几何路径是否成立。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidLosPath(const PathSearchContext& context, const PathState& state)
{
    GeometryValidityResult result;
    if (context.scene_query == nullptr || context.config == nullptr)
    {
        result.reason = GeometryValidityReason::InvalidContext;
        result.detail = "SceneQuery or AppConfig is null.";
        return result;
    }
    if (Length(state.current_direction) <= context.config->numeric_tolerance.eps_length)
    {
        result.reason = GeometryValidityReason::InvalidDirection;
        result.detail = "Current direction length is too small.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

/// <summary>
/// 检查反射候选面元命中是否有效。
/// </summary>
/// <param name="hit">面元命中结果。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidReflectionHitCandidate(const FaceHit& hit, const PathSearchContext& context)
{
    GeometryValidityResult result;
    if (!hit.hit)
    {
        result.reason = GeometryValidityReason::NoHit;
        result.detail = "Reflection candidate has no face hit.";
        return result;
    }
    if (hit.face_id < 0 || hit.face_id >= static_cast<int>(context.scene->faces.size()))
    {
        result.reason = GeometryValidityReason::NoHit;
        result.detail = "Reflection face id is out of range.";
        return result;
    }
    if (!context.scene->faces[hit.face_id].reflection_enabled)
    {
        result.reason = GeometryValidityReason::DisabledByFlags;
        result.detail = "Reflection disabled by face flags.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

/// <summary>
/// 检查透射候选面元命中是否有效。
/// </summary>
/// <param name="hit">面元命中结果。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidTransmissionHitCandidate(const FaceHit& hit, const PathSearchContext& context)
{
    GeometryValidityResult result;
    if (!hit.hit)
    {
        result.reason = GeometryValidityReason::NoHit;
        result.detail = "Transmission candidate has no face hit.";
        return result;
    }
    if (hit.face_id < 0 || hit.face_id >= static_cast<int>(context.scene->faces.size()))
    {
        result.reason = GeometryValidityReason::NoHit;
        result.detail = "Transmission face id is out of range.";
        return result;
    }
    const Face& face = context.scene->faces[hit.face_id];
    if (!face.transmission_enabled)
    {
        result.reason = GeometryValidityReason::DisabledByFlags;
        result.detail = "Transmission disabled by face flags.";
        return result;
    }
    if (!face.dual_side_material_resolved)
    {
        result.reason = GeometryValidityReason::InvalidMediumTransition;
        result.detail = "Dual-side material is unresolved.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

/// <summary>
/// 检查绕射候选是否有效。
/// </summary>
/// <param name="candidate">楔边候选结果。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidDiffractionCandidate(const WedgeCandidate& candidate)
{
    GeometryValidityResult result;
    if (candidate.wedge_id < 0)
    {
        result.reason = GeometryValidityReason::NoCandidate;
        result.detail = "No wedge candidate available.";
        return result;
    }
    if (candidate.length <= 0.0)
    {
        result.reason = GeometryValidityReason::DegenerateWedge;
        result.detail = "Wedge candidate length <= 0.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

/// <summary>
/// 检查新生成状态是否满足最小几何约束。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">新状态。</param>
/// <returns>结构化合法性结果。</returns>
GeometryValidityResult IsValidExpandedState(const PathSearchContext& context, const PathState& state)
{
    GeometryValidityResult result;
    if (!state.valid)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "Expanded state is marked invalid.";
        return result;
    }
    if (Length(state.current_direction) <= context.config->numeric_tolerance.eps_length)
    {
        result.reason = GeometryValidityReason::InvalidDirection;
        result.detail = "Expanded state direction length is too small.";
        return result;
    }
    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

} // namespace rt
