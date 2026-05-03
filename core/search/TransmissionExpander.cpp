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
        if (!context.scene_query->IsVisible(hit.position, context.rx_point, visibilityContext))
        {
            result.failure_reasons.push_back(GeometryValidityReason::VisibilityBlocked);
            continue;
        }

        PathState nextState = state;
        nextState.current_point = hit.position;
        nextState.current_direction = Normalize(Subtract(context.rx_point, hit.position));
        nextState.current_medium_id = mediumInfo.medium_out_id;
        nextState.last_interaction_type = InteractionType::Transmission;
        nextState.last_interaction_object_id = hit.object_id;
        nextState.last_hit_face_id = hit.face_id;
        nextState.last_interaction_normal = hit.normal;
        nextState.last_hit_front_side = mediumInfo.entered_from_front_side;
        nextState.accumulated_length += hit.distance;
        nextState.path_depth += 1;
        nextState.remaining_total_expansions -= 1;
        nextState.remaining_transmissions -= 1;
        nextState.ignored_face_id = hit.face_id;

        PathNode node;
        node.interaction_type = InteractionType::Transmission;
        node.object_id = hit.object_id;
        node.face_id = hit.face_id;
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

        result.next_states.push_back(nextState);
        break;
    }

    if (result.next_states.empty() && result.failure_reasons.empty())
    {
        result.failure_reasons.push_back(GeometryValidityReason::NoHit);
    }
    return result;
}

} // namespace rt
