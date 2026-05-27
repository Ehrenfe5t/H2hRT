// v8 Phase 1.2: SceneVisibilityBuilder 实现
#include "SceneVisibilityBuilder.h"

#include "../../core/common/math/Vec3.h"
#include "../../core/common/math/MathConstants.h"
#include "../../core/common/config/AppConfig.h"
#include "../../core/query/SceneQuery.h"
#include "../../core/scene/Scene.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <set>
#include <sstream>
#include <unordered_set>

namespace rt {

namespace {

// ── Fibonacci 半球采样 ──
// 生成 K 个在面元法向量所定义半球上均匀分布的方向
std::vector<Vec3> GenerateHemisphereSamples(const Vec3& normal, int K) {
    std::vector<Vec3> dirs; dirs.reserve(K);
    if (K <= 0) return dirs;
    const double phi = kPi * (3.0 - std::sqrt(5.0));  // golden angle
    for (int i = 0; i < K; ++i) {
        double y = 1.0 - (i / double(K - 1)) * 2.0;  // [-1, 1] along local Y
        double r = std::sqrt(std::max(0.0, 1.0 - y * y));
        double theta = phi * i;
        // 局部坐标 (Y-up = 法向量方向)
        Vec3 localDir = MakeVec3(std::cos(theta) * r, y, std::sin(theta) * r);
        // 只要上半球 (y > 0, 即法向量方向)
        if (localDir.y <= 0.0) continue;
        // 将局部坐标旋转到世界坐标 (法向量 = local Y)
        Vec3 worldDir = localDir;
        if (std::fabs(normal.x) > 0.001 || std::fabs(normal.z) > 0.001) {
            // 需要旋转: 以 Cross(Y_up, normal) 为轴
            Vec3 rotAxis = Normalize(Cross(MakeVec3(0.0, 1.0, 0.0), normal));
            double rotAngle = std::acos(Clamp(normal.y, -1.0, 1.0));
            double c = std::cos(rotAngle), s = std::sin(rotAngle);
            double t = 1.0 - c;
            double ax = rotAxis.x, ay = rotAxis.y, az = rotAxis.z;
            worldDir = MakeVec3(
                (t*ax*ax + c)*localDir.x + (t*ax*ay - s*az)*localDir.y + (t*ax*az + s*ay)*localDir.z,
                (t*ax*ay + s*az)*localDir.x + (t*ay*ay + c)*localDir.y + (t*ay*az - s*ax)*localDir.z,
                (t*ax*az - s*ay)*localDir.x + (t*ay*az + s*ax)*localDir.y + (t*az*az + c)*localDir.z);
        }
        worldDir = Normalize(worldDir);
        if (Length(worldDir) > 0.0) dirs.push_back(worldDir);
    }
    return dirs;
}

// ── 角度栅格方向生成 ──
Vec3 AngularGridDirection(int azi, int zen, int nAzi, int nZen) {
    double aziRad = (double(azi) / double(nAzi)) * 2.0 * kPi;
    double zenRad = (double(zen) / double(nZen)) * kPi;
    return MakeVec3(
        std::sin(zenRad) * std::cos(aziRad),
        std::cos(zenRad),
        std::sin(zenRad) * std::sin(aziRad));
}

// ── 两点间可见性快速检查 ──
bool QuickVisibility(const Point3& from, const Point3& to, const SceneQuery& query,
                     const AppConfig& config, int ignoreFaceA = -1, int ignoreFaceB = -1) {
    VisibilityQueryContext vc;
    vc.ignored_face_id = ignoreFaceA;
    vc.ignored_face_id2 = ignoreFaceB;
    vc.origin_offset_distance = config.numeric_tolerance.visibility_origin_offset;
    vc.target_shrink_distance = config.numeric_tolerance.visibility_target_shrink;
    return query.IsVisible(from, to, vc);
}

} // namespace

// ═══════════════════════════════════════════════════════════════════
//  BuildPVS
// ═══════════════════════════════════════════════════════════════════
bool SceneVisibilityBuilder::BuildPVS(Scene& scene, const SceneQuery& query, const AppConfig& config) {
    const int N = static_cast<int>(scene.faces.size());
    if (N == 0) return false;

    auto& pvs = scene.visibility.face_pvs;
    pvs.pvs_faces.clear();
    pvs.pvs_faces.resize(N);
    pvs.total_entries = 0;
    pvs.hemisphere_sample_count = kDefaultPVSHemisphereSamples;

    const double eps = config.numeric_tolerance.self_hit_ignore_distance;

    for (int i = 0; i < N; ++i) {
        const Face& face = scene.faces[i];
        if (face.degenerate || !face.reflection_enabled) continue;

        Point3 origin = Add(face.centroid, Scale(face.normal, eps * 10.0));
        auto samples = GenerateHemisphereSamples(face.normal, kDefaultPVSHemisphereSamples);

        std::unordered_set<int> visibleSet;
        for (const Vec3& dir : samples) {
            Ray ray; ray.origin = origin; ray.direction = dir;
            FaceQueryContext fqc;
            fqc.ignored_face_id = i;  // 忽略自身
            fqc.origin_ignore_distance = eps;
            FaceHit hit = query.QueryClosestFaceHit(ray, fqc);
            if (hit.hit && hit.face_id >= 0 && hit.face_id < N && hit.distance <= kDefaultPVSMaxDistance) {
                visibleSet.insert(hit.face_id);
            }
        }

        pvs.pvs_faces[i].assign(visibleSet.begin(), visibleSet.end());
        std::sort(pvs.pvs_faces[i].begin(), pvs.pvs_faces[i].end());
        pvs.total_entries += pvs.pvs_faces[i].size();
    }

    pvs.valid = true;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  BuildEdgeAdjacency
// ═══════════════════════════════════════════════════════════════════
bool SceneVisibilityBuilder::BuildEdgeAdjacency(Scene& scene, const SceneQuery& query, const AppConfig& config) {
    const int W = static_cast<int>(scene.wedges.size());
    if (W == 0) return false;

    auto& adj = scene.visibility.edge_adjacency;
    adj.adjacent_wedges.clear();
    adj.adjacent_wedges.resize(W);

    for (int i = 0; i < W; ++i) {
        const Wedge& wi = scene.wedges[i];
        if (!wi.diffractable) continue;

        for (int j = i + 1; j < W; ++j) {
            const Wedge& wj = scene.wedges[j];
            if (!wj.diffractable) continue;

            // 检查 wedge i 和 wedge j 之间的互相可见性
            if (QuickVisibility(wi.center_point, wj.center_point, query, config,
                                wi.positive_face_id, wi.negative_face_id) &&
                QuickVisibility(wj.center_point, wi.center_point, query, config,
                                wj.positive_face_id, wj.negative_face_id)) {
                adj.adjacent_wedges[i].push_back(j);
                adj.adjacent_wedges[j].push_back(i);
            }
        }
        adj.total_edges += adj.adjacent_wedges[i].size();
    }
    adj.total_edges /= 2;  // 每对计数两次

    adj.valid = true;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  BuildAngularGrid
// ═══════════════════════════════════════════════════════════════════
bool SceneVisibilityBuilder::BuildAngularGrid(Scene& scene, const SceneQuery& query, const AppConfig& config) {
    auto& grid = scene.visibility.angular_grid;
    grid.n_azimuth = kDefaultAngularGridAzi;
    grid.n_zenith = kDefaultAngularGridZen;
    int nCells = grid.CellCount();
    grid.cells.clear();
    grid.cells.resize(nCells);

    // 场景中心 (使用AABB中心作为探测射线起点)
    Point3 sceneCenter;
    if (scene.acceleration.face_acceleration.valid) {
        const AABB& sb = scene.acceleration.face_acceleration.scene_bounds;
        sceneCenter = MakeVec3((sb.min.x + sb.max.x) * 0.5,
                               (sb.min.y + sb.max.y) * 0.5,
                               (sb.min.z + sb.max.z) * 0.5);
    } else {
        sceneCenter = MakeVec3(0.0, 0.0, 0.0);
    }

    const double eps = config.numeric_tolerance.self_hit_ignore_distance;

    for (int azi = 0; azi < grid.n_azimuth; ++azi) {
        for (int zen = 0; zen < grid.n_zenith; ++zen) {
            Vec3 dir = AngularGridDirection(azi, zen, grid.n_azimuth, grid.n_zenith);
            Ray ray; ray.origin = sceneCenter; ray.direction = dir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance = eps;
            FaceHit hit = query.QueryClosestFaceHit(ray, fqc);

            int cellIdx = grid.CellIndex(azi, zen);
            if (hit.hit && hit.face_id >= 0) {
                grid.cells[cellIdx].push_back(hit.face_id);
                // 扩展: 包含该面元的 PVS (若已构建)
                if (scene.visibility.face_pvs.valid) {
                    for (int pvsFace : scene.visibility.face_pvs.GetVisibleFaces(hit.face_id)) {
                        grid.cells[cellIdx].push_back(pvsFace);
                    }
                }
            }
            // 去重
            std::sort(grid.cells[cellIdx].begin(), grid.cells[cellIdx].end());
            auto last = std::unique(grid.cells[cellIdx].begin(), grid.cells[cellIdx].end());
            grid.cells[cellIdx].erase(last, grid.cells[cellIdx].end());
        }
    }

    grid.valid = true;
    return true;
}

// ═══════════════════════════════════════════════════════════════════
//  BuildAll
// ═══════════════════════════════════════════════════════════════════
bool SceneVisibilityBuilder::BuildAll(Scene& scene, const SceneQuery& query, const AppConfig& config) {
    auto tStart = std::chrono::steady_clock::now();

    scene.visibility = SceneVisibilityData{};  // reset

    bool pvsOk = BuildPVS(scene, query, config);
    // Edge adjacency is only needed for Precise IM bidirectional search (§3.4).
    // SBR uses FindNearbyWedges (distance-based), not the adjacency graph.
    // Skipping O(W²) pairwise visibility checks saves 10-15 min for large scenes.
    bool edgeOk = true;
    if (config.em_solver.solver_mode != "Coverage") {
        edgeOk = BuildEdgeAdjacency(scene, query, config);
    }
    bool gridOk = BuildAngularGrid(scene, query, config);

    scene.visibility.valid = pvsOk && edgeOk && gridOk;

    auto tEnd = std::chrono::steady_clock::now();
    scene.visibility.build_time_seconds =
        std::chrono::duration<double>(tEnd - tStart).count();

    // 时间戳
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    char timeBuf[64];
    ctime_s(timeBuf, sizeof(timeBuf), &tt);
    scene.visibility.build_timestamp = timeBuf;
    if (!scene.visibility.build_timestamp.empty() && scene.visibility.build_timestamp.back() == '\n')
        scene.visibility.build_timestamp.pop_back();

    return scene.visibility.valid;
}

} // namespace rt
