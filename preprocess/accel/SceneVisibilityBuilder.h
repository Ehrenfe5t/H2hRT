// v8 Phase 1.2: 场景可见性预计算构建器
// PVS (面元-面元可见性) + Edge Adjacency (楔边间可见性) + Angular Grid (角度分区面元)
#pragma once

#include "../../core/scene/SceneVisibilityData.h"

namespace rt {

class SceneQuery;
struct AppConfig;
struct Scene;

/// <summary>
/// 场景可见性预计算构建器 — 离线执行, 结果存入 Scene::visibility
/// </summary>
class SceneVisibilityBuilder {
public:
    /// <summary>
    /// 构建全部预计算数据: PVS + Edge Adjacency + Angular Grid
    /// </summary>
    /// <param name="scene">场景 (读取面元/楔边, 写入 visibility 字段)</param>
    /// <param name="query">场景查询门面 (BVH遍历)</param>
    /// <param name="config">应用配置 (数值容差等)</param>
    /// <returns>true 表示构建成功</returns>
    static bool BuildAll(Scene& scene, const SceneQuery& query, const AppConfig& config);

    /// <summary>仅构建 PVS</summary>
    static bool BuildPVS(Scene& scene, const SceneQuery& query, const AppConfig& config);

    /// <summary>仅构建 Edge Adjacency</summary>
    static bool BuildEdgeAdjacency(Scene& scene, const SceneQuery& query, const AppConfig& config);

    /// <summary>仅构建 Angular Grid</summary>
    static bool BuildAngularGrid(Scene& scene, const SceneQuery& query, const AppConfig& config);

    // ── 配置参数 ──
    static constexpr int kDefaultPVSHemisphereSamples = 200;   // 每面元半球探测射线数
    static constexpr int kDefaultAngularGridAzi = 64;           // 角度栅格方位角分辨率
    static constexpr int kDefaultAngularGridZen = 32;           // 角度栅格天顶角分辨率
    static constexpr double kDefaultPVSMaxDistance = 100.0;     // PVS 最大可见距离 (m)
};

} // namespace rt
