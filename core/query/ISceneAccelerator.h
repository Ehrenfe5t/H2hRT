// v8 Phase 1: 场景加速器抽象接口 — CPU (FaceBVH) 与 GPU (OptiX/VulkanRT) 的统一抽象
// SceneQuery 通过此接口委托底层求交，上层寻径代码不感知加速器类型
// GPU 后端在 Phase 4 实现
#pragma once

#include "../scene/Scene.h"
#include <memory>
#include <string>
#include <vector>

namespace rt {

struct Ray;
struct FaceHit;
struct FaceQueryContext;
struct VisibilityQueryContext;

/// <summary>
/// 场景加速器抽象接口。
/// 实现类: CpuFaceBvhAccelerator (默认), OptiXSceneAccelerator (Phase 4 GPU)
/// </summary>
class ISceneAccelerator {
public:
    virtual ~ISceneAccelerator() = default;

    // ── 单射线查询 (CPU+GPU 通用) ──

    virtual FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const = 0;
    virtual bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const = 0;
    bool IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const {
        return !IsOccluded(start, end, ctx);
    }

    // ── 批量射线查询 (GPU 优化的入口点, CPU 后端默认循环调用单射线) ──

    virtual std::vector<FaceHit> QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const;
    virtual std::vector<bool> IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                               const VisibilityQueryContext& ctx) const;

    // ── 加速器元数据 ──

    virtual std::string BackendName() const = 0;
    virtual bool SupportsBatchQuery() const { return false; }
    virtual size_t MaxBatchSize() const { return 1; }

    // ── 场景数据生命周期 (GPU 后端用于上传/更新 GPU buffer) ──

    virtual void BuildFromScene(const Scene& scene) = 0;
    virtual void UpdateSceneFaces(const Scene& scene) {}  // 增量更新 (可选)

protected:
    ISceneAccelerator() = default;
    ISceneAccelerator(const ISceneAccelerator&) = delete;
    ISceneAccelerator& operator=(const ISceneAccelerator&) = delete;
};

// ── 默认批量实现 (CPU: 循环调用单射线) ──

inline std::vector<FaceHit> ISceneAccelerator::QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const {
    std::vector<FaceHit> results; results.reserve(rays.size());
    for (const auto& ray : rays) results.push_back(QueryClosestFaceHit(ray, ctx));
    return results;
}

inline std::vector<bool> ISceneAccelerator::IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                                              const VisibilityQueryContext& ctx) const {
    std::vector<bool> results; results.reserve(starts.size());
    for (size_t i = 0; i < starts.size(); ++i) results.push_back(IsOccluded(starts[i], ends[i], ctx));
    return results;
}

} // namespace rt
