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
    // v9 StageF: CPU fallback — segment-sphere check against Rx positions
    // GPU provides faster implementation via OptiX; CPU uses brute-force nearest check
    int N = static_cast<int>(seg_starts_flat.size() / 3);
    std::vector<std::vector<int>> results(N);
    if (!grid.rx_positions || grid.rx_count <= 0) return results;

    double r2 = grid.sphere_radius * grid.sphere_radius;
    for (int i = 0; i < N; ++i) {
        double sx = seg_starts_flat[i * 3 + 0];
        double sy = seg_starts_flat[i * 3 + 1];
        double sz = seg_starts_flat[i * 3 + 2];
        double ex = seg_ends_flat[i * 3 + 0];
        double ey = seg_ends_flat[i * 3 + 1];
        double ez = seg_ends_flat[i * 3 + 2];
        double abx = ex - sx, aby = ey - sy, abz = ez - sz;
        double ab2 = abx*abx + aby*aby + abz*abz;
        if (ab2 <= 0.0) {
            // Zero-length segment — check Rx at start point
            for (int ri = 0; ri < grid.rx_count; ++ri) {
                double dx = grid.rx_positions[ri].x - sx;
                double dy = grid.rx_positions[ri].y - sy;
                double dz = grid.rx_positions[ri].z - sz;
                if (dx*dx + dy*dy + dz*dz <= r2) { results[i].push_back(ri); }
            }
            continue;
        }
        double invAb2 = 1.0 / ab2;
        for (int ri = 0; ri < grid.rx_count; ++ri) {
            double rx = grid.rx_positions[ri].x;
            double ry = grid.rx_positions[ri].y;
            double rz = grid.rx_positions[ri].z;
            double apx = rx - sx, apy = ry - sy, apz = rz - sz;
            double t = (apx*abx + apy*aby + apz*abz) * invAb2;
            double cx, cy, cz;
            if (t <= 0.0) { cx = sx; cy = sy; cz = sz; }
            else if (t >= 1.0) { cx = ex; cy = ey; cz = ez; }
            else { cx = sx + t*abx; cy = sy + t*aby; cz = sz + t*abz; }
            double dx = rx - cx, dy = ry - cy, dz = rz - cz;
            if (dx*dx + dy*dy + dz*dz <= r2) { results[i].push_back(ri); break; } // first hit
        }
    }
    return results;
}

} // namespace rt
