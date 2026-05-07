// SbrEngine: forward ray-tracing coverage simulation (v4 C2)
// C2-A: Fixed reception sphere radius (user-specified, independent of ray count)
// C2-B: Rx spatial hash grid for O(K) per-segment lookup
// C2-C: OpenMP parallel ray tracing (rays are independent)

#include "SbrEngine.h"
#include "../common/math/Vec3.h"
#include "../common/math/MathConstants.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace rt {

namespace {

// Generate N uniformly distributed directions on unit sphere (Fibonacci golden-ratio spiral)
std::vector<Vec3> GenerateFibonacciRays(int N) {
    std::vector<Vec3> rays; rays.reserve(N);
    const double phi = kPi * (3.0 - std::sqrt(5.0));
    for (int i = 0; i < N; ++i) {
        double y = 1.0 - (i / double(N - 1)) * 2.0;
        double radius = std::sqrt(1.0 - y * y);
        double theta = phi * i;
        rays.push_back(MakeVec3(std::cos(theta) * radius, y, std::sin(theta) * radius));
    }
    return rays;
}

// Squared point-to-segment distance
double PointToSegmentDistSq(const Point3& p, const Point3& a, const Point3& b) {
    double abx = b.x - a.x, aby = b.y - a.y, abz = b.z - a.z;
    double apx = p.x - a.x, apy = p.y - a.y, apz = p.z - a.z;
    double ab2 = abx*abx + aby*aby + abz*abz;
    if (ab2 <= 0.0) return apx*apx + apy*apy + apz*apz;
    double t = (apx*abx + apy*aby + apz*abz) / ab2;
    if (t <= 0.0) return apx*apx + apy*apy + apz*apz;
    if (t >= 1.0) { double dx = p.x - b.x, dy = p.y - b.y, dz = p.z - b.z; return dx*dx + dy*dy + dz*dz; }
    double cx = a.x + t*abx, cy = a.y + t*aby, cz = a.z + t*abz;
    double dx = p.x - cx, dy = p.y - cy, dz = p.z - cz;
    return dx*dx + dy*dy + dz*dz;
}

// C2-B: Spatial hash grid for Rx points
struct RxHashGrid {
    double cellSize = 0.6;
    double sphereR = 0.3;
    std::unordered_map<uint64_t, std::vector<int>> cells;
    const std::vector<Point3>* rxPositions = nullptr;

    void Build(const std::vector<Point3>& rx, double radius) {
        rxPositions = &rx; sphereR = radius; cellSize = 2.0 * radius;
        cells.clear();
        for (int i = 0; i < static_cast<int>(rx.size()); ++i) {
            int cx = static_cast<int>(std::floor(rx[i].x / cellSize));
            int cy = static_cast<int>(std::floor(rx[i].y / cellSize));
            int cz = static_cast<int>(std::floor(rx[i].z / cellSize));
            uint64_t key = (uint64_t(cx & 0x1FFFFF) << 42) | (uint64_t(cy & 0x1FFFFF) << 21) | uint64_t(cz & 0x1FFFFF);
            cells[key].push_back(i);
        }
    }

    // Check segment against Rx in own cell + 26 neighbors
    void CheckSegment(const Point3& segStart, const Point3& segEnd,
                      std::vector<int>& hitRxIndices) const {
        double midX = (segStart.x + segEnd.x) * 0.5;
        double midY = (segStart.y + segEnd.y) * 0.5;
        double midZ = (segStart.z + segEnd.z) * 0.5;
        int cx = static_cast<int>(std::floor(midX / cellSize));
        int cy = static_cast<int>(std::floor(midY / cellSize));
        int cz = static_cast<int>(std::floor(midZ / cellSize));
        double r2 = sphereR * sphereR;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    uint64_t key = (uint64_t((cx+dx) & 0x1FFFFF) << 42)
                                 | (uint64_t((cy+dy) & 0x1FFFFF) << 21)
                                 | uint64_t((cz+dz) & 0x1FFFFF);
                    auto it = cells.find(key);
                    if (it == cells.end()) continue;
                    for (int rxi : it->second) {
                        double d2 = PointToSegmentDistSq((*rxPositions)[rxi], segStart, segEnd);
                        if (d2 <= r2) hitRxIndices.push_back(rxi);
                    }
                }
            }
        }
    }
};

// Reflection direction: reflected = incident - 2*(incident·normal)*normal
Vec3 ReflectDir(const Vec3& inc, const Vec3& n) {
    double d = Dot(inc, n);
    return MakeVec3(inc.x - 2.0*d*n.x, inc.y - 2.0*d*n.y, inc.z - 2.0*d*n.z);
}

} // namespace

SbrCoverageResult SbrEngine::Run(const SbrContext& context) const
{
    SbrCoverageResult result;
    if (!context.config || !context.scene || !context.scene_query) {
        result.trace_lines.push_back("SbrEngine: 上下文不完整"); return result;
    }

    const auto& cfg = context.config->sbr;
    const int N = cfg.ray_count;
    const int maxDepth = cfg.max_ray_depth;
    const double pwrThreshold = cfg.ray_power_threshold_linear;
    const double sphereR = cfg.rx_sphere_radius_m; // C2-A: fixed user-specified radius
    const int NRx = static_cast<int>(context.rx_grid.size());

    // C2-B: Build spatial hash grid
    RxHashGrid rxGrid;
    rxGrid.Build(context.rx_grid, sphereR);

    // Init per-Rx records
    result.rx_records.resize(NRx);
    for (int i = 0; i < NRx; ++i) {
        result.rx_records[i].rx_position = context.rx_grid[i];
        result.rx_records[i].rx_index = i;
    }
    result.total_rays = N;

    // Generate ray directions (Fibonacci sphere)
    std::vector<Vec3> rayDirs = GenerateFibonacciRays(N);

    std::ostringstream oss;
    oss << "SbrEngine: rays=" << N << " depth=" << maxDepth
        << " rxCount=" << NRx << " sphereR=" << sphereR << "m";
    result.trace_lines.push_back(oss.str());

    // C2-C: OpenMP parallel over rays
#ifdef RT_ENABLE_OPENMP
#pragma omp parallel for schedule(dynamic)
#endif
    for (int ri = 0; ri < N; ++ri)
    {
        Point3 curPt = context.tx_point;
        Vec3 curDir = rayDirs[ri];
        double curPwr = 1.0;
        int depth = 0;

        // Thread-local set to deduplicate Rx hits within one ray
        std::vector<bool> rxHitThisRay(NRx, false);

        while (depth <= maxDepth && curPwr > pwrThreshold)
        {
            Ray ray; ray.origin = curPt; ray.direction = curDir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance = context.config->numeric_tolerance.self_hit_ignore_distance;
            FaceHit hit = context.scene_query->QueryClosestFaceHit(ray, fqc);

            Point3 segEnd;
            if (hit.hit) segEnd = hit.position;
            else segEnd = Add(curPt, Scale(curDir, 1e6));

            // C2-B: Hash-based Rx check
            std::vector<int> hits;
            rxGrid.CheckSegment(curPt, segEnd, hits);
#ifdef RT_ENABLE_OPENMP
#pragma omp critical
#endif
            {
                for (int rxi : hits) {
                    if (!rxHitThisRay[rxi]) {
                        rxHitThisRay[rxi] = true;
                        GeometricPath path; path.path_id = rxi; path.valid = true;
                        PathNode txNode; txNode.interaction_type = InteractionType::Tx;
                        txNode.point = context.tx_point; txNode.valid = true;
                        path.nodes.push_back(txNode);
                        PathNode rxNode; rxNode.interaction_type = InteractionType::Rx;
                        rxNode.point = context.rx_grid[rxi]; rxNode.valid = true;
                        path.nodes.push_back(rxNode);
                        result.rx_records[rxi].paths.push_back(path);
                        result.rx_records[rxi].ray_hit_count++;
                        result.rx_records[rxi].total_power_linear += curPwr;
                    }
                }
            }

            if (!hit.hit) break;
            const Face& face = context.scene->faces[hit.face_id];
            if (face.reflection_enabled && depth < maxDepth) {
                // C7: Wedge diffraction — spawn diffracted rays from nearby wedges
                const auto& wedges = context.scene->wedges;
                int nWedges = static_cast<int>(wedges.size());
                if (nWedges > 0 && curPwr > pwrThreshold * 10.0) {
                    // Check a random subset of wedges near the hit point
                    int nCheck = std::min(nWedges, 16);
                    for (int wi = 0; wi < nCheck; ++wi) {
                        int wIdx = (ri * 17 + wi * 31) % nWedges; // pseudo-random
                        const Wedge& w = wedges[wIdx];
                        double distToWedge = Length(Subtract(w.center_point, hit.position));
                        if (distToWedge > 3.0) continue; // too far
                        Vec3 edgeDir = Normalize(w.direction);
                        if (Length(edgeDir) < 1e-9) continue;
                        // Keller cone: cos(beta0) = |hit_dir . edge_dir|
                        double cosBeta0 = std::fabs(Dot(curDir, edgeDir));
                        // Generate 4 diffracted ray directions around Keller cone
                        double beta0 = std::acos(std::min(1.0, cosBeta0));
                        Vec3 perpBase = Normalize(Cross(curDir, edgeDir));
                        if (Length(perpBase) < 1e-9) continue;
                        for (int d = 0; d < 4; ++d) {
                            double phi = (d * 0.5 + 0.25) * kPi; // 45, 135, 225, 315 deg
                            Vec3 diffDir = MakeVec3(
                                std::sin(beta0) * std::cos(phi) * perpBase.x + std::cos(beta0) * edgeDir.x + std::sin(beta0) * std::sin(phi) * (Cross(edgeDir, perpBase)).x,
                                std::sin(beta0) * std::cos(phi) * perpBase.y + std::cos(beta0) * edgeDir.y,
                                std::sin(beta0) * std::cos(phi) * perpBase.z + std::cos(beta0) * edgeDir.z);
                            // Trace diffraction sub-ray (quick: single segment)
                            Ray dRay; dRay.origin = w.center_point; dRay.direction = diffDir;
                            FaceHit dHit = context.scene_query->QueryClosestFaceHit(dRay, fqc);
                            Point3 dEnd = (dHit.hit) ? dHit.position : Add(w.center_point, Scale(diffDir, 5.0));
                            std::vector<int> dHits; rxGrid.CheckSegment(w.center_point, dEnd, dHits);
                            double diffPwr = curPwr * 0.05; // 5% of current power to diffraction
#ifdef RT_ENABLE_OPENMP
#pragma omp critical
#endif
                            {
                                for (int drxi : dHits) {
                                    if (!rxHitThisRay[drxi]) {
                                        rxHitThisRay[drxi] = true;
                                        GeometricPath dpath; dpath.path_id = drxi; dpath.valid = true;
                                        PathNode tn; tn.interaction_type = InteractionType::Tx; tn.point = context.tx_point; tn.valid = true; dpath.nodes.push_back(tn);
                                        PathNode dn; dn.interaction_type = InteractionType::Diffraction; dn.point = w.center_point; dn.valid = true; dpath.nodes.push_back(dn);
                                        PathNode rn; rn.interaction_type = InteractionType::Rx; rn.point = context.rx_grid[drxi]; rn.valid = true; dpath.nodes.push_back(rn);
                                        result.rx_records[drxi].paths.push_back(dpath);
                                        result.rx_records[drxi].ray_hit_count++;
                                        result.rx_records[drxi].total_power_linear += diffPwr;
                                    }
                                }
                            }
                        }
                    }
                }
                // Continue with reflection
                curDir = ReflectDir(curDir, hit.normal);
                curPt = hit.position; curPwr *= 0.5; depth++;
            } else break;
        }
    }

    // Finalize
    for (auto& rec : result.rx_records) {
        if (rec.ray_hit_count > 0) {
            rec.total_power_dBm = 10.0 * std::log10(rec.total_power_linear * 1000.0);
            result.active_rx_count++;
        }
    }
    result.succeeded = true;
    oss.str(""); oss << "SbrEngine: activeRx=" << result.active_rx_count << "/" << NRx;
    result.trace_lines.push_back(oss.str());
    return result;
}

} // namespace rt
