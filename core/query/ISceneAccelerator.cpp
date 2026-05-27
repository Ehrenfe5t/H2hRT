// v8: ISceneAccelerator default batch implementations
// Separate file to keep ISceneAccelerator.h lightweight (forward declarations only)

#include "ISceneAccelerator.h"
#include "SceneQuery.h"  // for Ray, FaceHit, Point3, etc.

namespace rt {

std::vector<FaceHit> ISceneAccelerator::QueryClosestFaceHitBatch(
    const std::vector<Ray>& rays, const FaceQueryContext& ctx) const
{
    std::vector<FaceHit> results; results.reserve(rays.size());
    for (const auto& ray : rays)
        results.push_back(QueryClosestFaceHit(ray, ctx));
    return results;
}

std::vector<bool> ISceneAccelerator::IsOccludedBatch(
    const std::vector<Point3>& starts, const std::vector<Point3>& ends,
    const VisibilityQueryContext& ctx) const
{
    std::vector<bool> results; results.reserve(starts.size());
    for (size_t i = 0; i < starts.size(); ++i)
        results.push_back(IsOccluded(starts[i], ends[i], ctx));
    return results;
}

std::vector<std::vector<int>> ISceneAccelerator::QueryRxHitsBatch(
    const std::vector<double>& seg_starts_flat,
    const std::vector<double>& seg_ends_flat,
    const RxGridQueryParams& grid) const
{
    // Default: empty (CPU path uses RxHashGrid directly in SbrEngine)
    int N = static_cast<int>(seg_starts_flat.size() / 3);
    return std::vector<std::vector<int>>(N);
}

} // namespace rt
