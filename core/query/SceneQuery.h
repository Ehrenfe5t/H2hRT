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
#include "ISceneAccelerator.h"

#include <memory>
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
    int ignored_face_id2 = -1;  // v8: 绕射可见性检查的第二忽略面
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
    int ignored_face_id2 = -1;  // v8: 绕射使用, 需同时忽略楔边的两个面
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
    /// 使用场景对象与配置构造查询门面 (默认 CPU BVH 加速)。
    /// </summary>
    SceneQuery(const Scene& scene, const AppConfig& config);

    /// <summary>
    /// 使用自定义加速器构造查询门面 (GPU OptiX 后端)。
    /// </summary>
    SceneQuery(const Scene& scene, const AppConfig& config,
               std::unique_ptr<ISceneAccelerator> accelerator);

    /// <summary>
    /// 查询射线正向最近面元命中。
    /// </summary>
    /// <param name="ray">输入射线。</param>
    /// <param name="context">局部查询约束。</param>
    /// <returns>最近有效面元命中结果。</returns>
    FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& context) const;

    /// <summary>[v7.3 SBR] 近远遍历+提前终止, 比全量快~30%, precise不可用。</summary>
    FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& context) const;

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
    std::vector<WedgeCandidate> QueryCandidateWedges(const Point3& origin, const WedgeQueryContext& context) const;

    // v8 GPU: batch query methods
    std::vector<FaceHit> QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const;
    std::vector<bool> IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                       const VisibilityQueryContext& ctx) const;
    std::vector<std::vector<int>> QueryRxHitsBatch(
        const std::vector<double>& seg_starts_flat,
        const std::vector<double>& seg_ends_flat,
        const ISceneAccelerator::RxGridQueryParams& grid) const;

    // v9 F-3: 后端查询诊断报告
    std::string GetBackendDiagnostics() const;

private:
    const Scene& scene_;
    const AppConfig& config_;
    std::unique_ptr<ISceneAccelerator> accelerator_;  // v8: GPU 可插拔后端
};

} // namespace rt
