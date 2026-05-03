// 文件目标：
// - 实现模块4批次6的绕射扩展器。
//
// 主要功能：
// - 基于模块2提供的 WedgeCandidate 查询结果发现绕射候选；
// - 检查候选可见性与最小楔边约束；
// - 生成新的绕射状态供 SearchEngine 继续处理。

#include "DiffractionExpander.h"

#include "StateSignatureBuilder.h"

#include <cmath>

namespace rt {

namespace {

Vec3 Subtract(const Point3& a, const Point3& b)
{
    Vec3 value;
    value.x = a.x - b.x;
    value.y = a.y - b.y;
    value.z = a.z - b.z;
    return value;
}

double Length(const Vec3& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

Vec3 Normalize(const Vec3& value)
{
    const double length = Length(value);
    if (length <= 0.0)
    {
        return Vec3{};
    }
    Vec3 result;
    result.x = value.x / length;
    result.y = value.y / length;
    result.z = value.z / length;
    return result;
}

} // namespace

/// <summary>
/// 执行一次绕射扩展。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化绕射扩展结果。</returns>
ExpanderResult ExpandDiffraction(const PathSearchContext& context, const PathState& state)
{
    ExpanderResult result;
    if (!state.allow_diffraction || state.remaining_diffractions <= 0)
    {
        result.failure_reasons.push_back(GeometryValidityReason::OutOfBudget);
        return result;
    }

    WedgeQueryContext queryContext;
    queryContext.ignored_wedge_id = state.last_hit_wedge_id;
    queryContext.recent_face_id = state.last_hit_face_id;
    queryContext.recent_wedge_id = state.last_hit_wedge_id;
    const std::vector<WedgeCandidate> candidates = context.scene_query->QueryCandidateWedges(state.current_point, queryContext);
    for (const WedgeCandidate& candidate : candidates)
    {
        const GeometryValidityResult candidateValidity = IsValidDiffractionCandidate(candidate);
        if (!candidateValidity.valid)
        {
            result.failure_reasons.push_back(candidateValidity.reason);
            continue;
        }

        VisibilityQueryContext visibilityContext;
        if (!context.scene_query->IsVisible(state.current_point, candidate.center_point, visibilityContext) ||
            !context.scene_query->IsVisible(candidate.center_point, context.rx_point, visibilityContext))
        {
            result.failure_reasons.push_back(GeometryValidityReason::VisibilityBlocked);
            continue;
        }

        PathState nextState = state;
        nextState.current_point = candidate.center_point;
        nextState.current_direction = Normalize(Subtract(context.rx_point, candidate.center_point));
        nextState.last_interaction_type = InteractionType::Diffraction;
        nextState.last_hit_wedge_id = candidate.wedge_id;
        nextState.accumulated_length += Length(Subtract(candidate.center_point, state.current_point));
        nextState.path_depth += 1;
        nextState.remaining_total_expansions -= 1;
        nextState.remaining_diffractions -= 1;

        PathNode node;
        node.interaction_type = InteractionType::Diffraction;
        node.wedge_id = candidate.wedge_id;
        node.point = candidate.center_point;
        node.direction = nextState.current_direction;
        node.segment_length_from_previous = Length(Subtract(candidate.center_point, state.current_point));
        node.valid = true;
        nextState.traversed_nodes.push_back(node);
        nextState.state_signature = BuildStateSignature(nextState, *context.config);
        nextState.valid = true;

        const GeometryValidityResult expandedValidity = IsValidExpandedState(context, nextState);
        if (!expandedValidity.valid)
        {
            result.failure_reasons.push_back(expandedValidity.reason);
            continue;
        }

        result.next_states.push_back(nextState);
        break;
    }

    if (result.next_states.empty() && result.failure_reasons.empty())
    {
        result.failure_reasons.push_back(GeometryValidityReason::NoCandidate);
    }
    return result;
}

} // namespace rt
