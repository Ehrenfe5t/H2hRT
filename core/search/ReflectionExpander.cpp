// 文件目标：
// - 实现模块4批次6的反射扩展器。
//
// 主要功能：
// - 通过镜像法在当前场景中寻找单次反射候选；
// - 对候选进行可见性和标志位检查；
// - 生成新的反射状态供 SearchEngine 继续处理。

#include "ReflectionExpander.h"

#include "StateSignatureBuilder.h"
#include "../common/math/Vec3.h"

#include <algorithm>
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Reflects a point across a plane defined by planePoint and planeNormal using
/// the Image Method (mirror-point symmetry), returning the mirrored position.
/// </summary>
Point3 MirrorPointAcrossPlane(const Point3& point, const Point3& planePoint, const Vec3& planeNormal)
{
    const Vec3 delta = Subtract(point, planePoint);
    const double signedDistance = Dot(delta, planeNormal);
    return Add(point, Scale(planeNormal, -2.0 * signedDistance));
}

/// <summary>
/// Scores a reflection candidate by combining Euclidean path length with an
/// angular penalty weighted by the surface-normal alignment of incoming/outgoing rays.
/// </summary>
double ComputeReflectionCandidateScore(const PathState& state, const FaceHit& hit, const Point3& rxPoint)
{
    const Vec3 hitToRx = Subtract(rxPoint, hit.position);
    const double rxDistance = Length(hitToRx);
    const Vec3 incoming = Normalize(Subtract(hit.position, state.current_point));
    const Vec3 outgoing = Normalize(hitToRx);
    const double normalIncoming = std::fabs(Dot(incoming, hit.normal));
    const double normalOutgoing = std::fabs(Dot(outgoing, hit.normal));
    const double angularPenalty = Clamp(1.0 - 0.5 * (normalIncoming + normalOutgoing), 0.0, 1.0);
    return hit.distance + rxDistance + angularPenalty * 5.0;
}

/// <summary>
/// Recursively traverses the face BVH (Bounding Volume Hierarchy) to collect
/// candidate face ids whose AABBs overlap the spatial search region.
/// </summary>
void CollectCandidateFaceIdsInRegion(
    const Scene& scene,
    int nodeIndex,
    const AABB& region,
    std::vector<int>& faceIds)
{
    const FaceBVH& bvh = scene.acceleration.face_acceleration.face_bvh;
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(bvh.nodes.size()))
    {
        return;
    }
    const FaceBVHNode& node = bvh.nodes[nodeIndex];

    // AABB-AABB overlap test
    const AABB& nb = node.bounds;
    const AABB& rb = region;
    if (!nb.valid || !rb.valid) return;
    if (nb.max.x < rb.min.x || nb.min.x > rb.max.x ||
        nb.max.y < rb.min.y || nb.min.y > rb.max.y ||
        nb.max.z < rb.min.z || nb.min.z > rb.max.z)
    {
        return;
    }

    if (node.is_leaf)
    {
        for (int i = 0; i < node.primitive_count; ++i)
        {
            int idx = node.start_index + i;
            if (idx >= 0 && idx < static_cast<int>(bvh.primitive_face_ids.size()))
            {
                faceIds.push_back(bvh.primitive_face_ids[idx]);
            }
        }
        return;
    }

    CollectCandidateFaceIdsInRegion(scene, node.left_child, region, faceIds);
    CollectCandidateFaceIdsInRegion(scene, node.right_child, region, faceIds);
}

/// <summary>
/// Computes an axis-aligned bounding box that encloses the Tx and Rx points
/// with a configurable margin, used as the spatial filter for the BVH face-gather step.
/// </summary>
AABB ComputeExtendedRegion(const Point3& tx, const Point3& rx, double margin)
{
    AABB region;
    region.min.x = (tx.x < rx.x ? tx.x : rx.x) - margin;
    region.min.y = (tx.y < rx.y ? tx.y : rx.y) - margin;
    region.min.z = (tx.z < rx.z ? tx.z : rx.z) - margin;
    region.max.x = (tx.x > rx.x ? tx.x : rx.x) + margin;
    region.max.y = (tx.y > rx.y ? tx.y : rx.y) + margin;
    region.max.z = (tx.z > rx.z ? tx.z : rx.z) + margin;
    region.valid = true;
    return region;
}

} // namespace

/// <summary>
/// 执行一次反射扩展。
/// </summary>
ExpanderResult ExpandReflection(const PathSearchContext& context, const PathState& state)
{
    ExpanderResult result;
    if (!state.allow_reflection || state.remaining_reflections <= 0)
    {
        result.failure_reasons.push_back(GeometryValidityReason::OutOfBudget);
        return result;
    }

    struct ReflectionCandidateState {
        PathState state;
        double score = 0.0;
    };
    std::vector<ReflectionCandidateState> acceptedStates;

    // B2-A: BVH spatial filter instead of iterating all faces
    const double margin = Length(Subtract(context.rx_point, state.current_point)) * 0.8; // v7.4 A5
    const AABB region = ComputeExtendedRegion(state.current_point, context.rx_point, margin);

    std::vector<int> candidateFaceIds;
    const FaceBVH& bvh = context.scene->acceleration.face_acceleration.face_bvh;
    if (bvh.valid)
    {
        CollectCandidateFaceIdsInRegion(*context.scene, 0, region, candidateFaceIds);
    }

    // Fallback: if BVH not available, iterate all faces
    const std::vector<Face>& facePool = context.scene->faces;
    const bool useBvhFilter = !candidateFaceIds.empty();
    const int totalChecks = useBvhFilter ? static_cast<int>(candidateFaceIds.size()) : static_cast<int>(facePool.size());

    for (int checkIdx = 0; checkIdx < totalChecks; ++checkIdx)
    {
        const int faceIdx = useBvhFilter ? candidateFaceIds[checkIdx] : checkIdx;
        if (faceIdx < 0 || faceIdx >= static_cast<int>(facePool.size())) continue;
        const Face& face = facePool[faceIdx];

        if (!face.reflection_enabled || face.degenerate)
        {
            continue;
        }

        const Point3 mirroredRx = MirrorPointAcrossPlane(context.rx_point, face.centroid, face.normal);
        const Vec3 candidateDirection = Normalize(Subtract(mirroredRx, state.current_point));
        if (Length(candidateDirection) <= context.config->numeric_tolerance.eps_length)
        {
            continue;
        }

        Ray ray;
        ray.origin = state.current_point;
        ray.direction = candidateDirection;
        FaceQueryContext queryContext;
        queryContext.ignored_face_id = state.ignored_face_id;
        queryContext.ignored_object_id = state.ignored_object_id; // v6 C5: 防止同Object自相交
        queryContext.origin_ignore_distance = context.config->numeric_tolerance.self_hit_ignore_distance;
        const FaceHit hit = context.scene_query->QueryClosestFaceHit(ray, queryContext);
        const GeometryValidityResult candidateValidity = IsValidReflectionHitCandidate(hit, context);
        if (!candidateValidity.valid || hit.face_id != face.face_id)
        {
            if (!candidateValidity.valid)
            {
                result.failure_reasons.push_back(candidateValidity.reason);
            }
            continue;
        }

        VisibilityQueryContext visibilityContext;
        visibilityContext.ignored_face_id = hit.face_id;
        if (!context.scene_query->IsVisible(state.current_point, hit.position, visibilityContext) ||
            !context.scene_query->IsVisible(hit.position, context.rx_point, visibilityContext))
        {
            result.failure_reasons.push_back(GeometryValidityReason::VisibilityBlocked);
            continue;
        }

        const Vec3 incoming = SafeNormalize(Subtract(hit.position, state.current_point));
        const Vec3 outgoing = SafeNormalize(Subtract(context.rx_point, hit.position));
        const double incomingNormalComponent = std::fabs(Dot(incoming, hit.normal));
        const double outgoingNormalComponent = std::fabs(Dot(outgoing, hit.normal));
        if (incomingNormalComponent <= 1.0e-3 || outgoingNormalComponent <= 1.0e-3)
        {
            result.failure_reasons.push_back(GeometryValidityReason::CandidateRejectedByControl);
            continue;
        }

        PathState nextState = state;
        nextState.current_point = hit.position;
        nextState.current_direction = SafeNormalize(Subtract(context.rx_point, hit.position));
        nextState.last_interaction_type = InteractionType::Reflection;
        nextState.last_interaction_object_id = hit.object_id;
        nextState.last_hit_face_id = hit.face_id;
        nextState.last_interaction_normal = hit.normal;
        nextState.accumulated_length += hit.distance;
        nextState.path_depth += 1;
        nextState.interaction_count += 1;
        nextState.remaining_total_expansions -= 1;
        nextState.remaining_reflections -= 1;
        nextState.ignored_face_id = hit.face_id;
        nextState.has_reflection = true;
        nextState.mixed_path_enabled = (nextState.has_reflection && nextState.has_transmission) || (nextState.has_reflection && nextState.has_diffraction) || (nextState.has_transmission && nextState.has_diffraction);
        nextState.clipped_by_control_rules = false;

        if (state.last_interaction_type != InteractionType::None &&
            state.last_interaction_type != InteractionType::Tx &&
            state.last_interaction_type != InteractionType::Reflection)
        {
            nextState.mechanism_switch_count += 1;
        }
        nextState.consecutive_same_interaction_count = (state.last_interaction_type == InteractionType::Reflection)
            ? (state.consecutive_same_interaction_count + 1)
            : 0;

        PathNode node;
        node.interaction_type = InteractionType::Reflection;
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

        ReflectionCandidateState candidateState;
        candidateState.state = nextState;
        candidateState.score = ComputeReflectionCandidateScore(state, hit, context.rx_point);
        acceptedStates.push_back(candidateState);
    }

    std::sort(acceptedStates.begin(), acceptedStates.end(),
        [](const ReflectionCandidateState& lhs, const ReflectionCandidateState& rhs)
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
