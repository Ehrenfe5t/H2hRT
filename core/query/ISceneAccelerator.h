// ISceneAccelerator
// Optional acceleration extension point. The v11 P2P baseline uses SceneQuery CPU FaceBVH directly.
// GPU/OptiX implementations are not active in the current main chain.
#pragma once

#include <vector>
#include <memory>
#include <string>

// Vec3.h defines Point3 via typedef, so include the full header here.
#include "../common/math/Vec3.h"

namespace rt {

// Forward declarations (full definitions in .cpp files only)
struct Ray;
struct FaceHit;
struct FaceQueryContext;
struct VisibilityQueryContext;
struct Scene;
struct AppConfig;

/// <summary>
/// Optional scene acceleration interface.
/// The active v11 P2P chain uses SceneQuery's CPU FaceBVH path directly.
/// </summary>
class ISceneAccelerator {
public:
    virtual ~ISceneAccelerator() = default;

    // Single ray queries.

    virtual FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const = 0;
    virtual bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const = 0;
    bool IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const {
        return !IsOccluded(start, end, ctx);
    }

    // Batch query extension points. Current v11 P2P does not require them.

    virtual std::vector<FaceHit> QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const;
    virtual std::vector<bool> IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                               const VisibilityQueryContext& ctx) const;

    // Batch segment-Rx collision query extension point for future coverage work.
    struct RxGridQueryParams {
        const Point3* rx_positions;     // CPU Rx positions [rx_count]
        const int* flat_cell_data;      // flat cell contents (Rx indices)
        const int* cell_offsets;        // [(nx*ny*nz)+1] prefix sum
        int flat_cell_data_size;        // size of flat_cell_data
        double cell_size, sphere_radius;
        double ox, oy, oz; int nx, ny, nz;
        int max_hits_per_seg;
        int rx_count;
    };
    virtual std::vector<std::vector<int>> QueryRxHitsBatch(
        const std::vector<double>& seg_starts_flat,
        const std::vector<double>& seg_ends_flat,
        const RxGridQueryParams& grid) const;

    // Accelerator metadata.

    virtual std::string BackendName() const = 0;
    virtual bool SupportsBatchQuery() const { return false; }
    virtual size_t MaxBatchSize() const { return 1; }
    // Capability flags for optional accelerator fallback decisions.
    virtual bool SupportsAllHits() const { return true; }
    virtual bool SupportsObjectFilter() const { return true; }
    virtual bool UsesDoublePrecision() const { return true; }

    // Backend query counters for diagnostics.
    mutable size_t closest_hit_queries = 0;
    mutable size_t all_hits_queries = 0;
    mutable size_t occlusion_queries = 0;
    void ResetQueryCounters() const {
        closest_hit_queries = all_hits_queries = occlusion_queries = 0;
    }

    // Scene data lifecycle.

    virtual void BuildFromScene(const Scene& scene) = 0;
    virtual void UpdateSceneFaces(const Scene& scene) {}  // incremental (optional)

protected:
    ISceneAccelerator() = default;
    ISceneAccelerator(const ISceneAccelerator&) = delete;
    ISceneAccelerator& operator=(const ISceneAccelerator&) = delete;
};

} // namespace rt
