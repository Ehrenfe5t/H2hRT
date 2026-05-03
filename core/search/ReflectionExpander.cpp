// 文件目标：
// - 实现模块4批次6的反射扩展器。
//
// 主要功能：
// - 通过镜像法在当前场景中寻找单次反射候选；
// - 对候选进行可见性和标志位检查；
// - 生成新的反射状态供 SearchEngine 继续处理。

#include "ReflectionExpander.h"

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

Point3 Add(const Point3& a, const Vec3& b)
{
    Point3 value;
    value.x = a.x + b.x;
    value.y = a.y + b.y;
    value.z = a.z + b.z;
    return value;
}

Vec3 Scale(const Vec3& value, double factor)
{
    Vec3 result;
    result.x = value.x * factor;
    result.y = value.y * factor;
    result.z = value.z * factor;
    return result;
}

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double Length(const Vec3& value)
{
    return std::sqrt(Dot(value, value));
}

Vec3 Normalize(const Vec3& value)
{
    const double length = Length(value);
    if (length <= 0.0)
    {
        return Vec3{};
    }
    return Scale(value, 1.0 / length);
}

Point3 MirrorPointAcrossPlane(const Point3& point, const Point3& planePoint, const Vec3& planeNormal)
{
    const Vec3 delta = Subtract(point, planePoint);
    const double signedDistance = Dot(delta, planeNormal);
    return Add(point, Scale(planeNormal, -2.0 * signedDistance));
}

} // namespace

/// <summary>
/// 执行一次反射扩展。
/// </summary>
/// <param name="context">搜索上下文。</param>
/// <param name="state">当前路径状态。</param>
/// <returns>结构化反射扩展结果。</returns>
ExpanderResult ExpandReflection(const PathSearchContext& context, const PathState& state)
{
    ExpanderResult result;
    if (!state.allow_reflection || state.remaining_reflections <= 0)
    {
        result.failure_reasons.push_back(GeometryValidityReason::OutOfBudget);
        return result;
    }

    for (const Face& face : context.scene->faces)
    {
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

        PathState nextState = state;
        nextState.current_point = hit.position;
        nextState.current_direction = Normalize(Subtract(context.rx_point, hit.position));
        nextState.last_interaction_type = InteractionType::Reflection;
        nextState.last_interaction_object_id = hit.object_id;
        nextState.last_hit_face_id = hit.face_id;
        nextState.last_interaction_normal = hit.normal;
        nextState.accumulated_length += hit.distance;
        nextState.path_depth += 1;
        nextState.remaining_total_expansions -= 1;
        nextState.remaining_reflections -= 1;
        nextState.ignored_face_id = hit.face_id;

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
