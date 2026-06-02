// v10 Iter1: 级联镜像法实现
// 核心算法: 反向连续镜像 → 正向递推求交 (O(k) per path)

#include "CascadeImageMethod.h"
#include "../query/SceneQuery.h"
#include <cmath>

namespace rt {

CascadeImageResult SolveCascadeReflection(
    const Point3& source,
    const Point3& target,
    const std::vector<int>& face_ids,
    const Scene& scene,
    SceneQuery& query)
{
    CascadeImageResult result;
    int m = static_cast<int>(face_ids.size());
    if (m == 0) {
        result.failure_reason = "empty face_ids";
        return result;
    }

    // 验证所有面元有效
    int N = static_cast<int>(scene.faces.size());
    for (int fid : face_ids) {
        if (fid < 0 || fid >= N) {
            result.failure_reason = "invalid face_id: " + std::to_string(fid);
            return result;
        }
        const Face& f = scene.faces[fid];
        if (!f.reflection_enabled || f.degenerate) {
            result.failure_reason = "face " + std::to_string(fid) + " not reflection-enabled or degenerate";
            return result;
        }
    }

    // ── 步骤1: 反向连续镜像 ──
    // mirrors[i] = target 经过 face_i ... face_{m-1} 连续镜像后的点
    // mirrors[m] = 原始 target
    std::vector<Point3> mirrors(m + 1);
    mirrors[m] = target;
    for (int i = m - 1; i >= 0; --i) {
        const Face& f = scene.faces[face_ids[i]];
        mirrors[i] = MirrorPointAcrossPlane(mirrors[i + 1], f.centroid, f.normal);
    }
    // mirrors[0] = target 经所有面元镜像后的最终点

    // ── 步骤2: 正向递推求交 ──
    result.reflection_points.resize(m);
    result.nodes.resize(m);
    Point3 src = source;
    double totalLen = 0.0;

    for (int i = 0; i < m; ++i) {
        // 向 mirrors[i] 连线 — mirrors[i] 已把 target 经剩余面元 (f_i..f_{m-1}) 镜像
        Vec3 dir = Normalize(Subtract(mirrors[i], src));
        if (Length(dir) < 1e-12) {
            result.failure_reason = "zero direction at step " + std::to_string(i);
            return result;
        }

        // BVH 射线求交
        Ray ray; ray.origin = src; ray.direction = dir;
        FaceQueryContext qc;
        qc.origin_ignore_distance = 1e-6;
        FaceHit hit = query.QueryClosestFaceHit(ray, qc);

        const Face& target_face = scene.faces[face_ids[i]];
        if (!hit.hit || hit.face_id != target_face.face_id) {
            result.failure_reason = "face mismatch at step " + std::to_string(i)
                + ": expected=" + std::to_string(face_ids[i])
                + ", got=" + std::to_string(hit.face_id);
            return result;
        }

        Point3 P = hit.position;
        double segLen = Length(Subtract(P, src));
        totalLen += segLen;

        // 记录反射点
        result.reflection_points[i] = P;

        // 构建 PathNode
        PathNode node;
        node.interaction_type = InteractionType::Reflection;
        node.face_id = target_face.face_id;
        node.object_id = target_face.object_id;
        node.point = P;
        node.direction = (i + 1 < m)
            ? Normalize(Subtract(mirrors[i + 1], P))  // 出射方向 = 向下一级镜像点
            : Normalize(Subtract(target, P));          // 最后一级: 向 target
        node.incident_direction = Normalize(Subtract(P, src));
        node.surface_normal = target_face.normal;
        node.segment_length_from_previous = segLen;
        node.valid = true;
        result.nodes[i] = node;

        // 下一段起点
        src = P;
    }

    // ── 步骤3: 最后一段 (最后一个反射点 → target) ──
    double finalSeg = Length(Subtract(target, result.reflection_points.back()));
    totalLen += finalSeg;

    result.total_length = totalLen;
    result.valid = true;
    return result;
}

} // namespace rt
