// v8: CPU BVH accelerator — wraps existing FaceBVH traversal, implements ISceneAccelerator
#pragma once

#include "ISceneAccelerator.h"
#include "../scene/Scene.h"
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
