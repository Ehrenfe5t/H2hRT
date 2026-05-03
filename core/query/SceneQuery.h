// 文件目标：
// - 定义模块2对模块4暴露的统一查询门面与相关高频返回结构。
//
// 主要功能：
// - 提供最近命中、全部命中、可见性与楔边候选查询；
// - 显式承载 FaceQueryContext / VisibilityQueryContext / WedgeQueryContext；
// - 保持模块4不直接依赖 BVH 节点等底层实现细节。

#pragma once

#include "../common/config/AppConfig.h"
#include "../scene/Scene.h"

#include <vector>

namespace rt {

/// <summary>
/// 射线结构。
/// </summary>
struct Ray {
    Point3 origin;
    Vec3 direction;
};

/// <summary>
/// 面元命中查询上下文。
/// </summary>
struct FaceQueryContext {
    int ignored_face_id = -1;
    int ignored_object_id = -1;
    bool ignore_origin_self_hit = true;
    double origin_ignore_distance = 1.0e-6;
    bool only_return_propagation_enabled_faces = false;
    bool require_dual_side_material_resolved = false;
};

/// <summary>
/// 可见性查询上下文。
/// </summary>
struct VisibilityQueryContext {
    int ignored_face_id = -1;
    int ignored_object_id = -1;
    bool ignore_origin_attached_face = true;
    bool ignore_target_attached_face = true;
    double origin_offset_distance = 1.0e-6;
    double target_shrink_distance = 1.0e-6;
};

/// <summary>
/// 楔边候选查询上下文。
/// </summary>
struct WedgeQueryContext {
    int ignored_wedge_id = -1;
    int ignored_object_id = -1;
    bool avoid_recent_wedge = true;
    bool avoid_adjacent_wedge_to_recent_face = true;
    int recent_face_id = -1;
    int recent_wedge_id = -1;
};

/// <summary>
/// 面元命中结果。
/// </summary>
struct FaceHit {
    bool hit = false;
    int face_id = -1;
    int object_id = -1;
    double distance = 0.0;
    Point3 position;
    Vec3 normal;
};

/// <summary>
/// 楔边候选结果。
/// </summary>
struct WedgeCandidate {
    int wedge_id = -1;
    int source_edge_id = -1;
    Point3 center_point;
    Vec3 direction;
    double length = 0.0;
    double wedge_angle_deg = 0.0;
    int positive_face_id = -1;
    int negative_face_id = -1;
};

/// <summary>
/// 场景统一查询门面。
/// </summary>
class SceneQuery {
public:
    /// <summary>
    /// 使用场景对象与配置构造查询门面。
    /// </summary>
    /// <param name="scene">静态场景对象引用。</param>
    /// <param name="config">统一应用配置对象引用。</param>
    SceneQuery(const Scene& scene, const AppConfig& config);

    /// <summary>
    /// 查询射线正向最近面元命中。
    /// </summary>
    /// <param name="ray">输入射线。</param>
    /// <param name="context">局部查询约束。</param>
    /// <returns>最近有效面元命中结果。</returns>
    FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& context) const;

    /// <summary>
    /// 查询射线正向全部面元命中。
    /// </summary>
    /// <param name="ray">输入射线。</param>
    /// <param name="context">局部查询约束。</param>
    /// <returns>按距离升序排列的全部有效命中。</returns>
    std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& context) const;

    /// <summary>
    /// 查询指定距离区间内的全部面元命中。
    /// </summary>
    /// <param name="ray">输入射线。</param>
    /// <param name="minDistance">最小距离。</param>
    /// <param name="maxDistance">最大距离。</param>
    /// <param name="context">局部查询约束。</param>
    /// <returns>区间内有效命中集合。</returns>
    std::vector<FaceHit> QueryFaceHitsInRange(
        const Ray& ray,
        double minDistance,
        double maxDistance,
        const FaceQueryContext& context) const;

    /// <summary>
    /// 判断两点开区间是否被有效面阻挡。
    /// </summary>
    /// <param name="start">线段起点。</param>
    /// <param name="end">线段终点。</param>
    /// <param name="context">可见性查询上下文。</param>
    /// <returns>true 表示存在阻挡；false 表示无遮挡。</returns>
    bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const;

    /// <summary>
    /// 判断两点是否可见。
    /// </summary>
    /// <param name="start">起点。</param>
    /// <param name="end">终点。</param>
    /// <param name="context">可见性查询上下文。</param>
    /// <returns>true 表示可见；false 表示不可见。</returns>
    bool IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const;

    /// <summary>
    /// 查询楔边候选集合。
    /// </summary>
    /// <param name="origin">当前参考点。</param>
    /// <param name="context">楔边查询上下文。</param>
    /// <returns>候选楔边集合。</returns>
    std::vector<WedgeCandidate> QueryCandidateWedges(const Point3& origin, const WedgeQueryContext& context) const;

private:
    const Scene& scene_;
    const AppConfig& config_;
};

} // namespace rt
