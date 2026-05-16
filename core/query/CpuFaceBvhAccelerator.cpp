// v8: CpuFaceBvhAccelerator — 委托现有 SceneQuery 实现 ISceneAccelerator
#include "CpuFaceBvhAccelerator.h"
#include "SceneQuery.h"

namespace rt {

CpuFaceBvhAccelerator::CpuFaceBvhAccelerator(const Scene& scene, const AppConfig& config)
    : scene_(scene), config_(config) {}

FaceHit CpuFaceBvhAccelerator::QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const {
    SceneQuery sq(scene_, config_);
    return sq.QueryClosestFaceHit(ray, ctx);
}
FaceHit CpuFaceBvhAccelerator::QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const {
    SceneQuery sq(scene_, config_);
    return sq.QueryClosestFaceHitFast(ray, ctx);
}
std::vector<FaceHit> CpuFaceBvhAccelerator::QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const {
    SceneQuery sq(scene_, config_);
    return sq.QueryAllFaceHits(ray, ctx);
}
std::vector<FaceHit> CpuFaceBvhAccelerator::QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const {
    SceneQuery sq(scene_, config_);
    return sq.QueryFaceHitsInRange(ray, tMin, tMax, ctx);
}
bool CpuFaceBvhAccelerator::IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const {
    SceneQuery sq(scene_, config_);
    return sq.IsOccluded(start, end, ctx);
}

} // namespace rt
