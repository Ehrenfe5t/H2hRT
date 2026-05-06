// 文件目标：
// - 实现模块2统一查询门面的第一版可用逻辑。
//
// 主要功能：
// - 基于批次3的 Face BVH 与 WedgeAcceleration 提供查询能力；
// - 支持最近命中、全部命中、范围命中、遮挡和可见性查询；
// - 显式处理自碰撞抑制与查询上下文约束。

#include "SceneQuery.h"

#include <algorithm>
#include <cmath>

namespace rt {

namespace {

Vec3 MakeVec3(double x, double y, double z)
{
    Vec3 value;
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

Vec3 Add(const Vec3& a, const Vec3& b)
{
    return MakeVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 Subtract(const Vec3& a, const Vec3& b)
{
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 Scale(const Vec3& value, double factor)
{
    return MakeVec3(value.x * factor, value.y * factor, value.z * factor);
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

bool IntersectRayAABB(const Ray& ray, const AABB& bounds, double& tMinOut, double& tMaxOut)
{
    if (!bounds.valid)
    {
        return false;
    }

    double tMin = -1.0e300;
    double tMax = 1.0e300;

    const double origins[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
    const double dirs[3] = { ray.direction.x, ray.direction.y, ray.direction.z };
    const double mins[3] = { bounds.min.x, bounds.min.y, bounds.min.z };
    const double maxs[3] = { bounds.max.x, bounds.max.y, bounds.max.z };

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::fabs(dirs[axis]) <= 1.0e-12)
        {
            if (origins[axis] < mins[axis] || origins[axis] > maxs[axis])
            {
                return false;
            }
            continue;
        }

        double invDir = 1.0 / dirs[axis];
        double t0 = (mins[axis] - origins[axis]) * invDir;
        double t1 = (maxs[axis] - origins[axis]) * invDir;
        if (t0 > t1)
        {
            std::swap(t0, t1);
        }
        if (t0 > tMin)
        {
            tMin = t0;
        }
        if (t1 < tMax)
        {
            tMax = t1;
        }
        if (tMin > tMax)
        {
            return false;
        }
    }

    tMinOut = tMin;
    tMaxOut = tMax;
    return true;
}

bool IntersectRayTriangle(const Ray& ray, const Point3& v0, const Point3& v1, const Point3& v2, double eps, double& tOut)
{
    const Vec3 edge1 = Subtract(v1, v0);
    const Vec3 edge2 = Subtract(v2, v0);
    const Vec3 pvec = MakeVec3(
        ray.direction.y * edge2.z - ray.direction.z * edge2.y,
        ray.direction.z * edge2.x - ray.direction.x * edge2.z,
        ray.direction.x * edge2.y - ray.direction.y * edge2.x);
    const double det = Dot(edge1, pvec);
    if (std::fabs(det) <= eps)
    {
        return false;
    }

    const double invDet = 1.0 / det;
    const Vec3 tvec = Subtract(ray.origin, v0);
    const double u = Dot(tvec, pvec) * invDet;
    if (u < -eps || u > 1.0 + eps)
    {
        return false;
    }

    const Vec3 qvec = MakeVec3(
        tvec.y * edge1.z - tvec.z * edge1.y,
        tvec.z * edge1.x - tvec.x * edge1.z,
        tvec.x * edge1.y - tvec.y * edge1.x);
    const double v = Dot(ray.direction, qvec) * invDet;
    if (v < -eps || u + v > 1.0 + eps)
    {
        return false;
    }

    const double t = Dot(edge2, qvec) * invDet;
    if (t <= eps)
    {
        return false;
    }

    tOut = t;
    return true;
}

bool AcceptFace(const Scene& scene, const Face& face, double distance, const FaceQueryContext& context)
{
    if (face.face_id == context.ignored_face_id)
    {
        return false;
    }
    if (face.object_id == context.ignored_object_id)
    {
        return false;
    }
    if (context.ignore_origin_self_hit && distance <= context.origin_ignore_distance)
    {
        return false;
    }
    if (context.only_return_propagation_enabled_faces && face.propagation_flags == FacePropagationNone)
    {
        return false;
    }
    if (context.require_dual_side_material_resolved && !face.dual_side_material_resolved)
    {
        return false;
    }
    static_cast<void>(scene);
    return true;
}

void CollectFaceHitsRecursive(
    const Scene& scene,
    const FaceBVH& bvh,
    int nodeIndex,
    const Ray& ray,
    const FaceQueryContext& context,
    double eps,
    std::vector<FaceHit>& hits)
{
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(bvh.nodes.size()))
    {
        return;
    }

    const FaceBVHNode& node = bvh.nodes[nodeIndex];
    double tMin = 0.0;
    double tMax = 0.0;
    if (!IntersectRayAABB(ray, node.bounds, tMin, tMax))
    {
        return;
    }

    if (node.is_leaf)
    {
        for (int i = 0; i < node.primitive_count; ++i)
        {
            const int primitiveIndex = node.start_index + i;
            if (primitiveIndex < 0 || primitiveIndex >= static_cast<int>(bvh.primitive_face_ids.size()))
            {
                continue;
            }

            const int faceId = bvh.primitive_face_ids[primitiveIndex];
            if (faceId < 0 || faceId >= static_cast<int>(scene.faces.size()))
            {
                continue;
            }

            const Face& face = scene.faces[faceId];
            const Point3& v0 = scene.vertices[face.vertex_index0];
            const Point3& v1 = scene.vertices[face.vertex_index1];
            const Point3& v2 = scene.vertices[face.vertex_index2];

            double distance = 0.0;
            if (!IntersectRayTriangle(ray, v0, v1, v2, eps, distance))
            {
                continue;
            }
            if (!AcceptFace(scene, face, distance, context))
            {
                continue;
            }

            FaceHit hit;
            hit.hit = true;
            hit.face_id = face.face_id;
            hit.object_id = face.object_id;
            hit.distance = distance;
            hit.position = Add(ray.origin, Scale(ray.direction, distance));
            hit.normal = face.normal;
            hits.push_back(hit);
        }
        return;
    }

    CollectFaceHitsRecursive(scene, bvh, node.left_child, ray, context, eps, hits);
    CollectFaceHitsRecursive(scene, bvh, node.right_child, ray, context, eps, hits);
}

} // namespace

/// <summary>
/// 使用场景对象与配置构造查询门面。
/// </summary>
/// <param name="scene">静态场景对象引用。</param>
/// <param name="config">统一应用配置对象引用。</param>
SceneQuery::SceneQuery(const Scene& scene, const AppConfig& config)
    : scene_(scene), config_(config)
{
}

/// <summary>
/// 查询射线正向最近面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>最近有效面元命中结果。</returns>
FaceHit SceneQuery::QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& context) const
{
    std::vector<FaceHit> hits = QueryAllFaceHits(ray, context);
    if (hits.empty())
    {
        return FaceHit{};
    }

    FaceHit closest = hits.front();
    for (const FaceHit& hit : hits)
    {
        if (!hit.hit)
        {
            continue;
        }
        if (!closest.hit || hit.distance < closest.distance)
        {
            closest = hit;
        }
    }
    return closest;
}

/// <summary>
/// 查询射线正向全部面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>按距离升序排列的全部有效命中。</returns>
std::vector<FaceHit> SceneQuery::QueryAllFaceHits(const Ray& ray, const FaceQueryContext& context) const
{
    std::vector<FaceHit> hits;
    if (!scene_.acceleration.face_acceleration.face_bvh.valid)
    {
        return hits;
    }

    CollectFaceHitsRecursive(
        scene_,
        scene_.acceleration.face_acceleration.face_bvh,
        0,
        ray,
        context,
        config_.numeric_tolerance.eps_intersection,
        hits);

    std::sort(hits.begin(), hits.end(),
        [](const FaceHit& lhs, const FaceHit& rhs)
        {
            return lhs.distance < rhs.distance;
        });
    return hits;
}

/// <summary>
/// 查询指定距离区间内的全部面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="minDistance">最小距离。</param>
/// <param name="maxDistance">最大距离。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>区间内有效命中集合。</returns>
std::vector<FaceHit> SceneQuery::QueryFaceHitsInRange(
    const Ray& ray,
    double minDistance,
    double maxDistance,
    const FaceQueryContext& context) const
{
    std::vector<FaceHit> allHits = QueryAllFaceHits(ray, context);
    std::vector<FaceHit> rangeHits;
    for (const FaceHit& hit : allHits)
    {
        if (hit.distance >= minDistance && hit.distance <= maxDistance)
        {
            rangeHits.push_back(hit);
        }
    }
    return rangeHits;
}

/// <summary>
/// 判断两点开区间是否被有效面阻挡。
/// </summary>
/// <param name="start">线段起点。</param>
/// <param name="end">线段终点。</param>
/// <param name="context">可见性查询上下文。</param>
/// <returns>true 表示存在阻挡；false 表示无遮挡。</returns>
bool SceneQuery::IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const
{
    const Vec3 delta = Subtract(end, start);
    const double length = Length(delta);
    if (length <= config_.numeric_tolerance.eps_length)
    {
        return false;
    }

    const Vec3 direction = Normalize(delta);
    Point3 rayOrigin = start;
    if (context.ignore_origin_attached_face)
    {
        rayOrigin = Add(rayOrigin, Scale(direction, context.origin_offset_distance));
    }
    Point3 rayEnd = end;
    if (context.ignore_target_attached_face)
    {
        rayEnd = Add(rayEnd, Scale(direction, -context.target_shrink_distance));
    }

    const double effectiveLength = Length(Subtract(rayEnd, rayOrigin));
    if (effectiveLength <= config_.numeric_tolerance.eps_length)
    {
        return false;
    }

    Ray ray;
    ray.origin = rayOrigin;
    ray.direction = direction;

    FaceQueryContext faceContext;
    faceContext.ignored_face_id = context.ignored_face_id;
    faceContext.ignored_object_id = context.ignored_object_id;
    faceContext.ignore_origin_self_hit = true;
    faceContext.origin_ignore_distance = config_.numeric_tolerance.self_hit_ignore_distance;

    const std::vector<FaceHit> hits = QueryFaceHitsInRange(ray, 0.0, effectiveLength, faceContext);
    return !hits.empty();
}

/// <summary>
/// 判断两点是否可见。
/// </summary>
/// <param name="start">起点。</param>
/// <param name="end">终点。</param>
/// <param name="context">可见性查询上下文。</param>
/// <returns>true 表示可见；false 表示不可见。</returns>
bool SceneQuery::IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const
{
    return !IsOccluded(start, end, context);
}

/// <summary>
/// 查询楔边候选集合。
/// </summary>
/// <param name="origin">当前参考点。</param>
/// <param name="context">楔边查询上下文。</param>
/// <returns>候选楔边集合。</returns>
std::vector<WedgeCandidate> SceneQuery::QueryCandidateWedges(const Point3& origin, const WedgeQueryContext& context) const
{
    std::vector<WedgeCandidate> candidates;

    for (const WedgeQueryRecord& record : scene_.acceleration.wedge_acceleration.wedge_query_records)
    {
        if (record.wedge_id == context.ignored_wedge_id)
        {
            continue;
        }
        if (context.avoid_recent_wedge && record.wedge_id == context.recent_wedge_id)
        {
            continue;
        }

        if (context.avoid_adjacent_wedge_to_recent_face &&
            (record.positive_face_id == context.recent_face_id || record.negative_face_id == context.recent_face_id))
        {
            continue;
        }

        const Vec3 delta = Subtract(record.center_point, origin);
        const double distanceToOrigin = Length(delta);
        if (distanceToOrigin > 50.0)
        {
            continue;
        }

        WedgeCandidate candidate;
        candidate.wedge_id = record.wedge_id;
        candidate.source_edge_id = record.source_edge_id;
        candidate.center_point = record.center_point;
        candidate.direction = record.direction;
        candidate.length = record.length;
        candidate.wedge_angle_deg = record.wedge_angle_deg;
        candidate.positive_face_id = record.positive_face_id;
        candidate.negative_face_id = record.negative_face_id;
        candidates.push_back(candidate);
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [&origin](const WedgeCandidate& lhs, const WedgeCandidate& rhs)
        {
            const double lhsDistance = Length(Subtract(lhs.center_point, origin));
            const double rhsDistance = Length(Subtract(rhs.center_point, origin));
            if (lhsDistance != rhsDistance)
            {
                return lhsDistance < rhsDistance;
            }
            return lhs.length > rhs.length;
        });

    if (static_cast<int>(candidates.size()) > config_.path_search.max_candidate_wedges)
    {
        candidates.resize(static_cast<std::size_t>(config_.path_search.max_candidate_wedges));
    }

    return candidates;
}

} // namespace rt
