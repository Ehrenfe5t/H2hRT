// v8 Phase 1: ISceneAccelerator — unified CPU/GPU acceleration interface
// Forward declarations enable .cu compilation without heavy C++ headers
#pragma once

#include <vector>
#include <memory>
#include <string>

// Vec3.h defines Point3 via typedef — include full header
// The .cu file uses a different translation unit; this header is included
// by both .cpp (MSVC) and .cu (nvcc). For nvcc, we provide compatible stubs.
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
/// Scene acceleration abstract interface.
/// Implementations: CpuFaceBvhAccelerator (default), OptiXSceneAccelerator (GPU)
/// </summary>
class ISceneAccelerator {
public:
    virtual ~ISceneAccelerator() = default;

    // ── Single ray queries (CPU + GPU common) ──

    virtual FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const = 0;
    virtual std::vector<FaceHit> QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const = 0;
    virtual bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const = 0;
    bool IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const {
        return !IsOccluded(start, end, ctx);
    }

    // ── Batch ray queries (GPU-optimized entry points) ──

    virtual std::vector<FaceHit> QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const;
    virtual std::vector<bool> IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                               const VisibilityQueryContext& ctx) const;

    // ── v8 GPU: batch segment-Rx collision query ──
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

    // ── Accelerator metadata ──

    virtual std::string BackendName() const = 0;
    virtual bool SupportsBatchQuery() const { return false; }
    virtual size_t MaxBatchSize() const { return 1; }

    // ── Scene data lifecycle ──

    virtual void BuildFromScene(const Scene& scene) = 0;
    virtual void UpdateSceneFaces(const Scene& scene) {}  // incremental (optional)

protected:
    ISceneAccelerator() = default;
    ISceneAccelerator(const ISceneAccelerator&) = delete;
    ISceneAccelerator& operator=(const ISceneAccelerator&) = delete;
};

} // namespace rt
