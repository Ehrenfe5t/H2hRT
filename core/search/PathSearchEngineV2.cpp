// v10: PathSearchEngineV2 — 移植自 RT.XD.SBR.CGAL 参考实现
// 核心算法: 递归深度优先 SBR (参考实现的 SbrFindGeometricPathSIMO)

#include "PathSearchEngineV2.h"
#include "PathSignatureBuilder.h"
#include "../query/SceneQuery.h"
#include "../common/material/MaterialDatabase.h"
#include <cstdio>
#include <cmath>
#include <unordered_set>
#include <vector>
#include <mutex>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace rt {

// ── 向量运算 (从 SbrEngine.cpp 移植) ──
static inline Vec3 ReflectDir(const Vec3& inc, const Vec3& n) {
    double d = Dot(inc, n);
    return MakeVec3(inc.x - 2.0*d*n.x, inc.y - 2.0*d*n.y, inc.z - 2.0*d*n.z);
}
static inline Vec3 RefractDir(const Vec3& inc, const Vec3& n, double cosI, double n2) {
    double sin2t = (1.0 - cosI*cosI) / (n2 * n2);
    if (sin2t >= 1.0) return ReflectDir(inc, n);
    double cosT = std::sqrt(1.0 - sin2t);
    return Normalize(MakeVec3(
        (inc.x + cosI*n.x)/n2 - n.x*cosT,
        (inc.y + cosI*n.y)/n2 - n.y*cosT,
        (inc.z + cosI*n.z)/n2 - n.z*cosT));
}

// ── 圆柱管 Rx 检测 (参考实现: Geometry3DIntersect.cpp:566-599) ──
static bool CylinderHitRx(const Point3& rayOrigin, const Vec3& rayDir,
                          double maxDist, double rxRadius,
                          const Point3& rxPoint)
{
    double ox = rxPoint.x - rayOrigin.x;
    double oy = rxPoint.y - rayOrigin.y;
    double oz = rxPoint.z - rayOrigin.z;
    double proj = ox*rayDir.x + oy*rayDir.y + oz*rayDir.z;
    if (proj < 0.0) return false;                     // Rx 必须在射线前方
    double dist2 = ox*ox + oy*oy + oz*oz;
    if (dist2 >= maxDist * maxDist) return false;      // 不能超过面元命中距离
    if (dist2 < 0.000025) return false;                // 距 Tx 太近 (5mm)
    double h2 = dist2 - proj * proj;                   // 垂直距离²
    return h2 <= rxRadius * rxRadius;
}

// ── RxHashGrid (从 SbrEngine.cpp 移植 — O(1) per ray segment) ──
struct RxHashGrid {
    double cellSize=0.6, sphereR=0.3;
    int nx=0, ny=0, nz=0;
    double ox=0, oy=0, oz=0;
    std::vector<std::vector<int>> flatCells;
    const std::vector<Point3>* rxPositions=nullptr;

    void Build(const std::vector<Point3>& rx, double radius) {
        rxPositions=&rx; sphereR=radius; cellSize=2.0*radius;
        if (rx.empty()) return;
        double xmin=rx[0].x, xmax=rx[0].x, ymin=rx[0].y, ymax=rx[0].y, zmin=rx[0].z, zmax=rx[0].z;
        for (auto& p:rx) {
            if(p.x<xmin)xmin=p.x; if(p.x>xmax)xmax=p.x;
            if(p.y<ymin)ymin=p.y; if(p.y>ymax)ymax=p.y;
            if(p.z<zmin)zmin=p.z; if(p.z>zmax)zmax=p.z;
        }
        ox=xmin-1.0; oy=ymin-1.0; oz=zmin-1.0;
        nx=std::max(1,(int)((xmax+1.0-ox)/cellSize)+1);
        ny=std::max(1,(int)((ymax+1.0-oy)/cellSize)+1);
        nz=std::max(1,(int)((zmax+1.0-oz)/cellSize)+1);
        if((size_t)nx*ny*nz > 50000000) { nx=464; ny=464; nz=464; }
        flatCells.assign((size_t)nx*ny*nz, {});
        for (int i=0; i<(int)rx.size(); ++i) {
            int cx=(int)((rx[i].x-ox)/cellSize), cy=(int)((rx[i].y-oy)/cellSize), cz=(int)((rx[i].z-oz)/cellSize);
            if (cx>=0&&cx<nx&&cy>=0&&cy<ny&&cz>=0&&cz<nz)
                flatCells[cx*ny*nz + cy*nz + cz].push_back(i);
        }
    }

    void CheckSegment(const Point3& a, const Point3& b, std::vector<int>& out) const {
        double r2=sphereR*sphereR;
        static thread_local std::vector<char> seen;
        static thread_local uint8_t gen=0;
        size_t NRx=rxPositions?rxPositions->size():0;
        if(seen.size()<NRx){seen.assign(NRx,0);gen=1;}
        if(++gen==0){std::fill(seen.begin(),seen.end(),0);gen=1;}
        double dx=a.x-b.x,dy=a.y-b.y,dz=a.z-b.z;
        double step=cellSize*0.5, len=std::sqrt(dx*dx+dy*dy+dz*dz);
        if(len<1e-9)return;
        int steps=std::max(2,(int)(len/step)+1);
        for (int s=0;s<=steps;++s) {
            double t=(double)s/steps, mx=a.x+(b.x-a.x)*t, my=a.y+(b.y-a.y)*t, mz=a.z+(b.z-a.z)*t;
            int cx=(int)((mx-ox)/cellSize), cy=(int)((my-oy)/cellSize), cz=(int)((mz-oz)/cellSize);
            int ci=cx*ny*nz+cy*nz+cz;
            if (cx<0||cx>=nx||cy<0||cy>=ny||cz<0||cz>=nz||ci<0||ci>=(int)flatCells.size()) continue;
            for (int ri:flatCells[ci]) {
                if(seen[ri]==gen)continue; seen[ri]=gen;
                double dxr=rxPositions->at(ri).x-mx, dyr=rxPositions->at(ri).y-my, dzr=rxPositions->at(ri).z-mz;
                if(dxr*dxr+dyr*dyr+dzr*dzr<=r2) out.push_back(ri);
            }
        }
    }
};

// ── DFS 状态 ──
struct SbrDfsState {
    Point3 pos; Vec3 dir;
    int depth, nRefl, nTrans, nDiff;
    std::vector<int> face_ids;
    std::vector<InteractionType> itypes;
    std::vector<Point3> hit_pts;
    std::vector<Vec3> inc_dirs, out_dirs, normals;
    std::vector<double> seg_lens;
};

// ── 递归 DFS (参考实现: SbrFindGeometricPathSIMO) ──
static void SbrDfs(const PathSearchContext& ctx, SbrDfsState& s,
                   const std::vector<Point3>& rxPos, const RxHashGrid& rxGrid,
                   double rxRadius, double freqHz, bool hasMat,
                   std::vector<GeometricPath>& outPaths,
                   std::mutex& pathMutex,
                   std::unordered_set<uint64_t>& globalSig)
{
    const auto& cfg = ctx.config->path_search;
    if (s.depth >= cfg.max_path_depth) return;

    // ── 面元求交 ──
    Ray ray; ray.origin = s.pos; ray.direction = s.dir;
    FaceQueryContext fqc;
    fqc.ignored_face_id = s.face_ids.empty() ? -1 : s.face_ids.back();
    fqc.ignore_origin_self_hit = true;
    fqc.origin_ignore_distance = 0.01;
    FaceHit hit = ctx.scene_query->QueryClosestFaceHit(ray, fqc);
    double maxDist = hit.hit ? hit.distance : 100.0;
    Point3 segEnd = hit.hit ? hit.position
        : MakeVec3(s.pos.x + s.dir.x*maxDist, s.pos.y + s.dir.y*maxDist, s.pos.z + s.dir.z*maxDist);

    // ── Rx 检测 (RxHashGrid O(1) + 圆柱管验证) ──
    std::vector<int> rxHits;
    rxGrid.CheckSegment(s.pos, segEnd, rxHits);
    for (int ri : rxHits) {
        if (!CylinderHitRx(s.pos, s.dir, maxDist, rxRadius, rxPos[ri]))
            continue;
        VisibilityQueryContext vc;
        if (!ctx.scene_query->IsVisible(s.pos, rxPos[ri], vc))
            continue;

        GeometricPath gp;
        gp.is_los = (s.depth == 0);
        gp.contains_transmission = (s.nTrans > 0);
        PathNode txNode; txNode.interaction_type = InteractionType::Tx;
        txNode.point = ctx.tx_point; txNode.valid = true;
        gp.nodes.push_back(txNode);
        for (size_t k = 0; k < s.face_ids.size(); ++k) {
            PathNode pn; pn.interaction_type = s.itypes[k];
            pn.face_id = s.face_ids[k]; pn.wedge_id = -1;
            pn.point = s.hit_pts[k]; pn.valid = true;
            pn.incident_direction = (k < s.inc_dirs.size()) ? s.inc_dirs[k] : Vec3{};
            pn.direction = (k < s.out_dirs.size()) ? s.out_dirs[k] : Vec3{};
            pn.surface_normal = (k < s.normals.size()) ? s.normals[k] : Vec3{};
            pn.segment_length_from_previous = (k < s.seg_lens.size()) ? s.seg_lens[k] : 0.0;
            if (s.itypes[k] == InteractionType::Transmission && pn.face_id >= 0
                && pn.face_id < (int)ctx.scene->faces.size()) {
                const Face& tf = ctx.scene->faces[pn.face_id];
                pn.transmission_semantic_complete = tf.dual_side_material_resolved;
                bool ff = Dot(pn.incident_direction, tf.normal) < 0;
                pn.medium_in_id  = ff ? tf.front_medium_id : tf.back_medium_id;
                pn.medium_out_id = ff ? tf.back_medium_id  : tf.front_medium_id;
                pn.front_medium_id = tf.front_medium_id;
                pn.back_medium_id  = tf.back_medium_id;
                pn.entered_from_front_side = ff;
            } else if (s.itypes[k] == InteractionType::Transmission) {
                pn.transmission_semantic_complete = true;
                pn.medium_in_id = 0; pn.medium_out_id = 1;
            }
            gp.nodes.push_back(pn);
        }
        PathNode rxNode; rxNode.interaction_type = InteractionType::Rx;
        rxNode.point = rxPos[ri]; rxNode.valid = true;
        rxNode.segment_length_from_previous = Length(Subtract(rxPos[ri],
            s.hit_pts.empty() ? ctx.tx_point : s.hit_pts.back()));
        gp.nodes.push_back(rxNode);
        gp.total_length = 0;
        for (size_t j = 1; j < gp.nodes.size(); ++j)
            gp.total_length += Length(Subtract(gp.nodes[j].point, gp.nodes[j-1].point));
        gp.valid = true;
        gp.path_signature = BuildPathSignature(gp, *ctx.config);
        {
            std::lock_guard<std::mutex> lk(pathMutex);
            if (globalSig.insert(gp.path_signature).second)
                outPaths.push_back(gp);
        }
    }

    if (!hit.hit) return;
    const Face& face = ctx.scene->faces[hit.face_id];
    if (face.degenerate) return;

    double cosI = std::fabs(Dot(s.dir, hit.normal));
    if (cosI < 1e-4) cosI = 1e-4;

    // ── 反射分支 ──
    if (face.reflection_enabled && s.nRefl < cfg.max_reflection_count) {
        double segLen = Length(Subtract(hit.position, s.pos));
        SbrDfsState nr = s;
        nr.dir = ReflectDir(s.dir, hit.normal);
        nr.pos = Add(hit.position, Scale(nr.dir, 0.01));
        nr.depth++; nr.nRefl++;
        nr.face_ids.push_back(hit.face_id);
        nr.itypes.push_back(InteractionType::Reflection);
        nr.hit_pts.push_back(hit.position);
        nr.inc_dirs.push_back(s.dir);
        nr.out_dirs.push_back(nr.dir);
        nr.normals.push_back(hit.normal);
        nr.seg_lens.push_back(segLen);
        SbrDfs(ctx, nr, rxPos, rxGrid, rxRadius, freqHz, hasMat, outPaths, pathMutex, globalSig);
    }

    // ── 透射分支 (参考实现: 根据 face 两侧材料查 real εr) ──
    if (face.transmission_enabled && face.dual_side_material_resolved
        && s.nTrans < cfg.max_transmission_count && hasMat) {
        // 使用 face 的真实 εr (通过 MaterialDatabase 的 ITU 插值)
        double epsR = face.surface_eps_r;
        double n2 = std::sqrt(std::max(1.0, epsR));
        double sinT2 = (1.0 - cosI * cosI) / (n2 * n2);
        if (sinT2 < 1.0) {  // 非 TIR
            double segLen = Length(Subtract(hit.position, s.pos));
            SbrDfsState nt = s;
            nt.dir = RefractDir(s.dir, hit.normal, cosI, n2);
            nt.pos = Add(hit.position, Scale(nt.dir, 0.01));
            nt.depth++; nt.nTrans++;
            nt.face_ids.push_back(hit.face_id);
            nt.itypes.push_back(InteractionType::Transmission);
            nt.hit_pts.push_back(hit.position);
            nt.inc_dirs.push_back(s.dir);
            nt.out_dirs.push_back(nt.dir);
            nt.normals.push_back(hit.normal);
            nt.seg_lens.push_back(segLen);
            SbrDfs(ctx, nt, rxPos, rxGrid, rxRadius, freqHz, hasMat, outPaths, pathMutex, globalSig);
        }
    }
}

// ═══════════════════════════════════════════════════════════
// 主入口
// ═══════════════════════════════════════════════════════════

SearchEngineResult PathSearchEngineV2::Run(const PathSearchContext& context) const
{
    SearchEngineResult result;
    result.uses_real_scene_query = true;

    if (!context.config || !context.scene || !context.scene_query) {
        result.trace_lines.push_back("V2 aborted: incomplete context");
        return result;
    }

    const auto& scfg = context.config->path_search;
    const auto& sbrCfg = context.config->sbr;
    int N_rays = sbrCfg.ray_count;
    double freqHz = context.config->em_solver.frequency_hz;
    bool hasMat = context.material_db && !context.material_db->empty();
    double rxRadius = std::max(0.5, sbrCfg.rx_sphere_radius_m); // 0.5m min

    // ── 收集 Rx ──
    std::vector<Point3> rxPos;
    for (const auto& rx : scfg.rx_list)
        rxPos.push_back(MakeVec3(rx.x, rx.y, rx.z));
    if (rxPos.empty())
        rxPos.push_back(context.rx_point);
    int RxCount = (int)rxPos.size();

    std::fprintf(stderr, "[V2] SBR-DFS: rays=%d R<=%d T<=%d D<=%d depth<=%d rx=%d r=%.2f\n",
                 N_rays, scfg.max_reflection_count, scfg.max_transmission_count,
                 scfg.max_diffraction_count, scfg.max_path_depth, RxCount, rxRadius);

    // ── Fibonacci 球面初始射线 ──
    std::vector<Vec3> initDirs(N_rays);
    double phi = 3.14159265358979323846 * (3.0 - std::sqrt(5.0));
    for (int i = 0; i < N_rays; ++i) {
        double y = 1.0 - (i / (N_rays - 1.0)) * 2.0;
        double r = std::sqrt(1.0 - y * y);
        double th = phi * i;
        initDirs[i] = MakeVec3(r * std::cos(th), y, r * std::sin(th));
    }

    // ── RxHashGrid 加速 (O(1) per ray segment) ──
    RxHashGrid rxGrid;
    rxGrid.Build(rxPos, rxRadius);

    // ── 多线程 DFS ──
    std::vector<GeometricPath> allPaths;
    std::unordered_set<uint64_t> globalSig;
    std::mutex pathMutex;

    int nThreads = 1;
#ifdef _OPENMP
    nThreads = omp_get_max_threads();
#endif

    std::vector<std::vector<GeometricPath>> threadPaths(nThreads);
    std::vector<std::unordered_set<uint64_t>> threadSig(nThreads);

#pragma omp parallel for schedule(dynamic, 500)
    for (int i = 0; i < N_rays; ++i) {
        int tid = 0;
#ifdef _OPENMP
        tid = omp_get_thread_num();
#endif
        SbrDfsState s;
        s.pos = context.tx_point;
        s.dir = initDirs[i];
        s.depth = 0; s.nRefl = 0; s.nTrans = 0; s.nDiff = 0;

        SbrDfs(context, s, rxPos, rxGrid, rxRadius, freqHz, hasMat,
               threadPaths[tid], pathMutex, threadSig[tid]);
    }

    // ── 合并去重 ──
    for (int t = 0; t < nThreads; ++t) {
        for (auto& p : threadPaths[t]) {
            if (globalSig.insert(p.path_signature).second) {
                p.path_id = (int)allPaths.size();
                allPaths.push_back(p);
            }
        }
    }

    result.path_set.paths = std::move(allPaths);
    result.generated_state_count = N_rays;
    result.accepted_state_count = (int)result.path_set.paths.size();
    result.succeeded = true;

    std::fprintf(stderr, "[V2] Done: paths=%zu\n", result.path_set.paths.size());
    char buf[128];
    snprintf(buf, sizeof(buf), "V2-SBR-DFS: %zu paths", result.path_set.paths.size());
    result.trace_lines.push_back(buf);

    return result;
}

} // namespace rt
