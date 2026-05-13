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
#include "../common/math/Vec3.h"

#include <algorithm>
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Scores a transmission candidate based on path length, incidence-angle
/// penalty, and a penalty for degenerate medium transitions (same in/out medium).
/// </summary>
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

        // B6-A: Snell refraction instead of pointing at Rx
        Vec3 incidentDir = Normalize(Subtract(hit.position, state.current_point));
        Vec3 txDir;
        if (context.material_db && !context.material_db->empty())
        {
            // Get refractive indices from material database
            const Face& txFace = context.scene->faces[hit.face_id];
            double n1 = 1.0; // default: air
            double n2 = 1.0;
            if (!txFace.front_material_name.empty())
            {
                auto p1 = context.material_db->QueryByName(txFace.front_material_name, context.config->em_solver.frequency_hz);
                auto p2 = context.material_db->QueryByName(txFace.back_material_name, context.config->em_solver.frequency_hz);
                if (mediumInfo.entered_from_front_side) { n1 = std::sqrt(std::max(1.0, p1.epsilon_r)); n2 = std::sqrt(std::max(1.0, p2.epsilon_r)); }
                else { n1 = std::sqrt(std::max(1.0, p2.epsilon_r)); n2 = std::sqrt(std::max(1.0, p1.epsilon_r)); }
            }
            txDir = SnellRefract(incidentDir, hit.normal, n1, n2);
            if (Length(txDir) <= 0.0) continue; // v7.4 A7: TIR→拒绝候选, 不回退Rx方向
        }
        else
        {
            continue; // v7.4 A7: 无材质DB→拒绝, 不产生物理无效路径
        }

        PathState nextState = state;
        nextState.current_point = hit.position;
        nextState.current_direction = txDir;
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
        nextState.mixed_path_enabled = (nextState.has_reflection && nextState.has_transmission) || (nextState.has_reflection && nextState.has_diffraction) || (nextState.has_transmission && nextState.has_diffraction);
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
        node.incident_direction = incidentDir; // v7.4 B15: 供EM层Fresnel计算恢复入射角
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
    const std::size_t keepLimit = std::min<std::size_t>(acceptedStates.size(), 8U);
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
