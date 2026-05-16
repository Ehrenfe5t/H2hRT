// v8: CPU BVH加速器 — 封装现有FaceBVH遍历逻辑, 实现 ISceneAccelerator 接口
// Phase 4 GPU 时替换为 OptiXSceneAccelerator
#pragma once

#include "ISceneAccelerator.h"
#include "../common/config/AppConfig.h"

namespace rt {

class CpuFaceBvhAccelerator : public ISceneAccelerator {
public:
    CpuFaceBvhAccelerator(const Scene& scene, const AppConfig& config);

    FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const override;
    FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const override;
    std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const override;
    std::vector<FaceHit> QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const override;
    bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const override;

    std::string BackendName() const override { return "CPU_FaceBVH"; }

    void BuildFromScene(const Scene& scene) override {}
    // 查询构建状态
    bool IsBuilt() const { return built_; }

private:
    const Scene& scene_;
    const AppConfig& config_;
    bool built_ = true;
};

} // namespace rt
