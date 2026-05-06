// 文件目标：
// - 实现模块4批次6的几何合法性检查逻辑。
//
// 主要功能：
// - 为 LOS、反射、透射、绕射扩展输出统一判定结果；
// - 在批次6范围内优先保证失败原因显式可追溯；
// - 为后续剪枝与统计提供结构化失败原因源。

#include "GeometryValidity.h"
#include "../common/math/Vec3.h"

#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Returns true if points a and b are within eps distance component-wise,
/// used to detect revisiting a near-identical interaction location.
/// </summary>
bool IsSamePoint(const Point3& a, const Point3& b, double eps)
{
    return std::fabs(a.x - b.x) <= eps &&
           std::fabs(a.y - b.y) <= eps &&
           std::fabs(a.z - b.z) <= eps;
}

/// <summary>
/// Scans the traversed node list of a PathState to check whether any node
/// has the specified interaction type.
/// </summary>
bool ContainsInteraction(const PathState& state, InteractionType t)
{
    for (const auto& n : state.traversed_nodes)
        if (n.interaction_type == t) return true;
    return false;
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
    if (state.path_depth > state.interaction_count)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "path_depth exceeds interaction_count bookkeeping.";
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
    if (state.traversed_nodes.empty())
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "LOS check requires at least a Tx node in traversed_nodes.";
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
    if (hit.distance <= context.config->numeric_tolerance.eps_length)
    {
        result.reason = GeometryValidityReason::CandidateRejectedByControl;
        result.detail = "Reflection hit distance is too small for a stable candidate.";
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
    if (!face.transmission_semantic_complete)
    {
        result.reason = GeometryValidityReason::InvalidMediumTransition;
        result.detail = "Transmission semantic source is incomplete at scene layer.";
        return result;
    }
    if (hit.distance <= context.config->numeric_tolerance.eps_length)
    {
        result.reason = GeometryValidityReason::CandidateRejectedByControl;
        result.detail = "Transmission hit distance is too small for a stable candidate.";
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
    if (Length(candidate.direction) <= 0.0)
    {
        result.reason = GeometryValidityReason::CandidateRejectedByControl;
        result.detail = "Wedge candidate direction is too small.";
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
    if (state.path_depth > context.config->path_search.max_path_depth)
    {
        result.reason = GeometryValidityReason::PathDepthExceeded;
        result.detail = "Expanded state path_depth exceeds path_search.max_path_depth.";
        return result;
    }
    if (state.interaction_count > context.config->path_search.max_path_depth)
    {
        result.reason = GeometryValidityReason::OutOfBudget;
        result.detail = "interaction_count exceeds max_path_depth budget.";
        return result;
    }
    if (state.traversed_nodes.empty())
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "Expanded state has empty traversed_nodes.";
        return result;
    }

    const PathNode& lastNode = state.traversed_nodes.back();
    if (!lastNode.valid)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "Last traversed node is invalid.";
        return result;
    }

    if (lastNode.segment_length_from_previous < 0.0)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "segment_length_from_previous is negative.";
        return result;
    }

    // B1: IsInvalidImmediateSequence removed. Any interaction sequence is allowed.
    // Per-mechanism budget counters (remaining_reflections/transmissions/diffractions)
    // are the primary control mechanism.

    int maxConsecutiveSame = context.config->path_search.max_consecutive_same_interaction;
    if (maxConsecutiveSame > 0 && state.consecutive_same_interaction_count > maxConsecutiveSame)
    {
        result.reason = GeometryValidityReason::CandidateRejectedByControl;
        result.detail = "Consecutive same interaction count exceeds configurable limit (max_consecutive_same_interaction="
            + std::to_string(maxConsecutiveSame) + ").";
        return result;
    }

    // B1: mixed_path_enabled is now a statistics-only flag. No longer blocks any path.
    // B1: Diffraction-based mixed paths are now fully allowed.

    if (state.mechanism_switch_count > state.path_depth)
    {
        result.reason = GeometryValidityReason::InvalidState;
        result.detail = "mechanism_switch_count bookkeeping exceeds path_depth.";
        return result;
    }

    if (lastNode.interaction_type == InteractionType::Transmission)
    {
        if (!lastNode.transmission_semantic_complete)
        {
            result.reason = GeometryValidityReason::InvalidMediumTransition;
            result.detail = "Transmission node is missing complete semantic chain fields.";
            return result;
        }
        if (lastNode.medium_in_id < 0 || lastNode.medium_out_id < 0)
        {
            result.reason = GeometryValidityReason::InvalidMediumTransition;
            result.detail = "Transmission node medium_in/out are invalid.";
            return result;
        }
        if (lastNode.medium_in_id == lastNode.medium_out_id)
        {
            result.reason = GeometryValidityReason::CandidateRejectedByControl;
            result.detail = "Transmission node does not change medium context.";
            return result;
        }
    }

    for (std::size_t i = 0; i + 1U < state.traversed_nodes.size(); ++i)
    {
        const PathNode& node = state.traversed_nodes[i];
        if (node.interaction_type == InteractionType::Reflection && node.face_id >= 0 && node.face_id == lastNode.face_id)
        {
            result.reason = GeometryValidityReason::DuplicateInteractionLoop;
            result.detail = "Repeated reflection on the same face is blocked by A2-S1 control layer.";
            return result;
        }
        if (node.interaction_type == InteractionType::Transmission && node.face_id >= 0 && node.face_id == lastNode.face_id)
        {
            result.reason = GeometryValidityReason::DuplicateInteractionLoop;
            result.detail = "Repeated transmission on the same face is blocked by A2-S1 control layer.";
            return result;
        }
        if (node.interaction_type == InteractionType::Diffraction && node.wedge_id >= 0 && node.wedge_id == lastNode.wedge_id)
        {
            result.reason = GeometryValidityReason::DuplicateInteractionLoop;
            result.detail = "Repeated diffraction on the same wedge is blocked by A2-S1 control layer.";
            return result;
        }
        if (node.interaction_type != InteractionType::Tx &&
            IsSamePoint(node.point, lastNode.point, context.config->numeric_tolerance.eps_deduplicate))
        {
            result.reason = GeometryValidityReason::DuplicateInteractionLoop;
            result.detail = "Expanded state revisits a near-identical interaction point.";
            return result;
        }
    }

    if (ContainsInteraction(state, InteractionType::Scattering))
    {
        result.reason = GeometryValidityReason::CandidateRejectedByControl;
        result.detail = "Scattering is not part of A2-S1 control-layer rollout.";
        return result;
    }

    result.valid = true;
    result.reason = GeometryValidityReason::Ok;
    return result;
}

} // namespace rt
