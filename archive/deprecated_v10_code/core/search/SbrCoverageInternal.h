// v11: Internal types shared between SbrEngine.cpp and SbrLegacyCoverage.cpp.
// NOT part of the public API — for internal use within core/search/ only.
#pragma once

#include "../common/math/Vec3.h"
#include <cstdint>
#include <vector>

namespace rt {
namespace sbr_internal {

// ── Rx spatial hash grid for O(1) per-segment Rx queries ──
struct RxHashGrid {
    std::vector<int>         cellOffsets;
    std::vector<int>         cellRxIds;
    std::vector<Point3>      rxPositions;
    double                   cellSize    = 0.6;
    double                   invCellSize = 1.0;
    int                      gridNX = 0, gridNY = 0, gridNZ = 0;
    int                      gridNYNZ = 0;
    Point3                   gridMin;

    void Build(const std::vector<Point3>& rx, double radius);
    int  CellIndex(int cx, int cy, int cz) const;
    void CheckSegment(const Point3& a, const Point3& b, std::vector<int>& out) const;
    void CheckSegmentWithRadius(const Point3& a, const Point3& b, double radius, std::vector<int>& out) const;
};

// ── Tube-coupled wedge record ──
struct SegmentWedgeCoupling {
    int wedgeId  = -1;
    int wedgeIdx = -1;
    double segmentT = 0.0;
    double wedgeT   = 0.0;
    double distance = 0.0;
    Point3 closestOnSegment;
    Point3 closestOnWedge;
};

// ── Per-ray state for wavefront depth-layered batch processing ──
struct alignas(64) SbrRayState {
    double ox, oy, oz;
    double dx, dy, dz;
    double curPwr;
    int cr, ct, cd;
    int noNewHit;
    uint32_t rng;
    int rayIdx;
    int last_wedge_id;
    bool alive;
    int diff_paths_stored;
    char _pad[3];
};
static_assert(sizeof(SbrRayState) <= 128, "SbrRayState too large");

// ── Point-to-point trace state ──
struct SbrPointToPointState {
    Point3 current_point;
    Vec3   current_direction;
    double current_power = 1.0;
    int    remaining_reflections = 0;
    int    remaining_transmissions = 0;
    int    depth = 0;
    int    last_face_id = -1;
    std::vector<int> node_indices;
};

} // namespace sbr_internal
} // namespace rt
