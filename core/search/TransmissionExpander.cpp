// 文件目标：
// - 实现模块4批次6的透射扩展器。
//
// 主要功能：
// - 沿当前状态指向 Rx 的连线查找透射面元候选；
// - 显式解析透射前后介质切换；
// - 生成新的透射状态供 SearchEngine 继续处理。

#include "TransmissionExpander.h"

#include "ResolveMediumTransition.h"
#include "StateSignatureBuilder.h"

#include <algorithm>
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

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double ComputeTransmissionCandidateScore(
    const PathState& state,
    const FaceHit& hit,
    const Point3& rxPoint,
    const MediumTransitionInfo& mediumInfo)
{
    const Vec3 hitToRx = Subtract(rxPoint, hit.position);
    const double rxDistance = Length(hitToRx);
    const Vec3 incoming = Normalize(Subtract(hit.position, state.current_point));
    const double incidencePenalty = 1.0 - std::fabs(Dot(incoming, hit.normal));
    const double mediumPenalty = (mediumInfo.medium_in_id == mediumInfo.medium_out_id) ? 0.5 : 0.0;
    return hit.distance + rxDistance + incidencePenalty * 3.0 + mediumPenalty;
}

} // namespace

/// <summary>
/// 执行一次透射扩展。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化透射扩展结果。</returns>
ExpanderResult ExpandTransmission(const PathSearchContext& context, const PathState& state)
{
    ExpanderResult result;
    if (!state.allow_transmission || state.remaining_transmissions <= 0)
    {
        result.failure_reasons.push_back(GeometryValidityReason::OutOfBudget);
        return result;
    }

    const Vec3 towardRx = Subtract(context.rx_point, state.current_point);
    const double maxDistance = Length(towardRx);
    if (maxDistance <= context.config->numeric_tolerance.eps_length)
    {
        result.failure_reasons.push_back(GeometryValidityReason::InvalidDirection);
        return result;
    }

    struct TransmissionCandidateState {
        PathState state;
        double score = 0.0;
    };
    std::vector<TransmissionCandidateState> acceptedStates;

    Ray ray;
    ray.origin = state.current_point;
    ray.direction = Normalize(towardRx);
    FaceQueryContext queryContext;
    queryContext.ignored_face_id = state.ignored_face_id;
    queryContext.require_dual_side_material_resolved = true;
    queryContext.origin_ignore_distance = context.config->numeric_tolerance.self_hit_ignore_distance;
    const std::vector<FaceHit> hits = context.scene_query->QueryFaceHitsInRange(ray, 0.0, maxDistance, queryContext);
    for (const FaceHit& hit : hits)
    {
        const GeometryValidityResult candidateValidity = IsValidTransmissionHitCandidate(hit, context);
        if (!candidateValidity.valid)
        {
            result.failure_reasons.push_back(candidateValidity.reason);
            continue;
        }

        const MediumTransitionInfo mediumInfo = ResolveMediumTransition(state, hit, context);
        if (!mediumInfo.valid)
        {
            result.failure_reasons.push_back(GeometryValidityReason::InvalidMediumTransition);
            continue;
        }

        VisibilityQueryContext visibilityContext;
        visibilityContext.ignored_face_id = hit.face_id;
        visibilityContext.ignored_object_id = hit.object_id;
        if (!context.scene_query->IsVisible(hit.position, context.rx_point, visibilityContext))
        {
            result.failure_reasons.push_back(GeometryValidityReason::VisibilityBlocked);
            continue;
        }

        const Vec3 incoming = Normalize(Subtract(hit.position, state.current_point));
        const double normalComponent = std::fabs(Dot(incoming, hit.normal));
        if (normalComponent <= 1.0e-3)
        {
            result.failure_reasons.push_back(GeometryValidityReason::CandidateRejectedByControl);
            continue;
        }

        PathState nextState = state;
        nextState.current_point = hit.position;
        nextState.current_direction = Normalize(Subtract(context.rx_point, hit.position));
        nextState.current_medium_id = mediumInfo.medium_out_id;
        nextState.last_interaction_type = InteractionType::Transmission;
        nextState.last_interaction_object_id = hit.object_id;
        nextState.last_hit_face_id = hit.face_id;
        nextState.last_medium_in_id = mediumInfo.medium_in_id;
        nextState.last_medium_out_id = mediumInfo.medium_out_id;
        nextState.last_front_medium_id = mediumInfo.front_medium_id;
        nextState.last_back_medium_id = mediumInfo.back_medium_id;
        nextState.last_interaction_normal = hit.normal;
        nextState.last_hit_front_side = mediumInfo.entered_from_front_side;
        nextState.last_transmission_semantic_complete = mediumInfo.dual_side_semantic_complete;
        nextState.accumulated_length += hit.distance;
        nextState.path_depth += 1;
        nextState.interaction_count += 1;
        nextState.remaining_total_expansions -= 1;
        nextState.remaining_transmissions -= 1;
        nextState.ignored_face_id = hit.face_id;
        nextState.has_transmission = true;
        nextState.mixed_path_enabled = state.has_reflection && !state.has_diffraction;
        nextState.clipped_by_control_rules = false;

        if (state.last_interaction_type != InteractionType::None &&
            state.last_interaction_type != InteractionType::Tx &&
            state.last_interaction_type != InteractionType::Transmission)
        {
            nextState.mechanism_switch_count += 1;
        }
        nextState.consecutive_same_interaction_count = (state.last_interaction_type == InteractionType::Transmission)
            ? (state.consecutive_same_interaction_count + 1)
            : 0;

        PathNode node;
        node.interaction_type = InteractionType::Transmission;
        node.object_id = hit.object_id;
        node.face_id = hit.face_id;
        node.medium_in_id = mediumInfo.medium_in_id;
        node.medium_out_id = mediumInfo.medium_out_id;
        node.front_medium_id = mediumInfo.front_medium_id;
        node.back_medium_id = mediumInfo.back_medium_id;
        node.front_material_id = mediumInfo.front_material_id;
        node.back_material_id = mediumInfo.back_material_id;
        node.entered_from_front_side = mediumInfo.entered_from_front_side;
        node.transmission_semantic_complete = mediumInfo.dual_side_semantic_complete;
        node.point = hit.position;
        node.direction = nextState.current_direction;
        node.surface_normal = hit.normal;
        node.segment_length_from_previous = hit.distance;
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

        TransmissionCandidateState candidateState;
        candidateState.state = nextState;
        candidateState.score = ComputeTransmissionCandidateScore(state, hit, context.rx_point, mediumInfo);
        acceptedStates.push_back(candidateState);
    }

    std::sort(acceptedStates.begin(), acceptedStates.end(),
        [](const TransmissionCandidateState& lhs, const TransmissionCandidateState& rhs)
        {
            return lhs.score < rhs.score;
        });
    const std::size_t keepLimit = std::min<std::size_t>(acceptedStates.size(), 4U);
    for (std::size_t i = 0; i < keepLimit; ++i)
    {
        result.next_states.push_back(acceptedStates[i].state);
    }

    if (result.next_states.empty() && result.failure_reasons.empty())
    {
        result.failure_reasons.push_back(GeometryValidityReason::NoHit);
    }
    return result;
}

} // namespace rt
