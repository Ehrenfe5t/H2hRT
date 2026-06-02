// SbrEngine: forward ray-tracing coverage simulation (v4 C2 + v5 D7 + v5 precision)
// v5 precision: Monte Carlo transmission + Fresnel caching + wedge throttling + power normalization

#include "SbrEngine.h"
#include "PathSignatureBuilder.h"
#include "../query/SbrGpuPipeline.h"  // v8 GPU megakernel
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <omp.h>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rt {

namespace {

// ── Fast xorshift RNG (thread-safe, no global state) ──
inline uint32_t XorShift32(uint32_t& state) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
inline double RandDouble(uint32_t& state) {
    return XorShift32(state) / 4294967296.0; // [0, 1)
}

// ── Fibonacci sphere ──
std::vector<Vec3> GenerateFibonacciRays(int N) {
    std::vector<Vec3> rays; rays.reserve(N);
    if (N <= 0) return rays;
    if (N == 1) { rays.push_back(MakeVec3(0.0, 1.0, 0.0)); return rays; }
    const double phi = kPi * (3.0 - std::sqrt(5.0));
    for (int i = 0; i < N; ++i) {
        double y = 1.0 - (i / double(N - 1)) * 2.0;
        double r = std::sqrt(1.0 - y * y), th = phi * i;
        rays.push_back(MakeVec3(std::cos(th)*r, y, std::sin(th)*r));
    }
    return rays;
}

double PointToSegmentDistSq(const Point3& p, const Point3& a, const Point3& b) {
    double abx=b.x-a.x, aby=b.y-a.y, abz=b.z-a.z;
    double apx=p.x-a.x, apy=p.y-a.y, apz=p.z-a.z;
    double ab2=abx*abx+aby*aby+abz*abz;
    if (ab2<=0.0) return apx*apx+apy*apy+apz*apz;
    double t=(apx*abx+apy*aby+apz*abz)/ab2;
    if (t<=0.0) return apx*apx+apy*apy+apz*apz;
    if (t>=1.0) { double dx=p.x-b.x,dy=p.y-b.y,dz=p.z-b.z; return dx*dx+dy*dy+dz*dz; }
    double cx=a.x+t*abx,cy=a.y+t*aby,cz=a.z+t*abz;
    double dx=p.x-cx,dy=p.y-cy,dz=p.z-cz;
    return dx*dx+dy*dy+dz*dz;
}

// ── Rx spatial grid ──
// v8: 密集3D网格替代unordered_map — O(1)直接索引, 无哈希开销
struct RxHashGrid {
    double cellSize=0.6, sphereR=0.3;
    int nx=0, ny=0, nz=0;            // grid dimensions
    double ox=0, oy=0, oz=0;          // grid origin
    std::vector<std::vector<int>> flatCells; // flat[nx*ny*nz]
    const std::vector<Point3>* rxPositions=nullptr;

    void Build(const std::vector<Point3>& rx, double radius) {
        rxPositions=&rx; sphereR=radius; cellSize=2.0*radius;
        if (rx.empty()) return;
        // Compute grid bounds from Rx positions
        double xmin=rx[0].x, xmax=rx[0].x, ymin=rx[0].y, ymax=rx[0].y, zmin=rx[0].z, zmax=rx[0].z;
        for (auto& p:rx) {
            if(p.x<xmin)xmin=p.x; if(p.x>xmax)xmax=p.x;
            if(p.y<ymin)ymin=p.y; if(p.y>ymax)ymax=p.y;
            if(p.z<zmin)zmin=p.z; if(p.z>zmax)zmax=p.z;
        }
        ox=xmin-1.0; oy=ymin-1.0; oz=zmin-1.0;
        nx=static_cast<int>((xmax+1.0-ox)/cellSize)+1;
        ny=static_cast<int>((ymax+1.0-oy)/cellSize)+1;
        nz=static_cast<int>((zmax+1.0-oz)/cellSize)+1;
        size_t total = static_cast<size_t>(nx)*ny*nz;
        if (total > 100000000) { total=100000000; nx=464; ny=464; nz=464; } // safety cap
        flatCells.assign(total, {});
        for (int i=0; i<static_cast<int>(rx.size()); ++i) {
            int cx=static_cast<int>((rx[i].x-ox)/cellSize);
            int cy=static_cast<int>((rx[i].y-oy)/cellSize);
            int cz=static_cast<int>((rx[i].z-oz)/cellSize);
            if (cx>=0&&cx<nx&&cy>=0&&cy<ny&&cz>=0&&cz<nz)
                flatCells[cx*ny*nz + cy*nz + cz].push_back(i);
        }
    }
    inline int CellIndex(int cx, int cy, int cz) const {
        if (cx<0||cx>=nx||cy<0||cy>=ny||cz<0||cz>=nz) return -1;
        return cx*ny*nz + cy*nz + cz;
    }
    void CheckSegment(const Point3& a,const Point3& b,std::vector<int>& out) const {
        double r2=sphereR*sphereR;
        double dxb=std::abs(b.x-a.x), dyb=std::abs(b.y-a.y), dzb=std::abs(b.z-a.z);
        bool spansMultiCell = (dxb>2.0*cellSize || dyb>2.0*cellSize || dzb>2.0*cellSize);
        // v8: thread_local seen mask — O(1)去重替代 O(K²) 线性扫描
        static thread_local std::vector<char> seenMask;
        size_t NRx = rxPositions ? rxPositions->size() : 0;
        if (seenMask.size() < NRx) seenMask.resize(NRx, 0);
        // 使用generation counter避免每步memset
        static thread_local uint8_t genCounter = 0;
        genCounter = (genCounter + 1) & 0xFF;
        if (genCounter == 0) { std::fill(seenMask.begin(), seenMask.end(), 0); genCounter = 1; }

        auto queryCell = [&](double mx, double my, double mz) {
            int cx=static_cast<int>((mx-ox)/cellSize);
            int cy=static_cast<int>((my-oy)/cellSize);
            int cz=static_cast<int>((mz-oz)/cellSize);
            for (int dx=-1;dx<=1;++dx) for (int dy=-1;dy<=1;++dy) for (int dz=-1;dz<=1;++dz) {
                int ci = CellIndex(cx+dx, cy+dy, cz+dz);
                if (ci<0) continue;
                for (int rxi: flatCells[ci]) {
                    if (seenMask[rxi] == genCounter) continue;
                    if (PointToSegmentDistSq((*rxPositions)[rxi],a,b)<=r2) {
                        seenMask[rxi] = genCounter;
                        out.push_back(rxi);
                    }
                }
            }
        };

        if (!spansMultiCell) {
            // 短线段: 中点查询 (覆盖99%情况, 避免多点采样开销)
            double mx=(a.x+b.x)*0.5, my=(a.y+b.y)*0.5, mz=(a.z+b.z)*0.5;
            queryCell(mx, my, mz);
        } else {
            // 长线段: 多点采样防止跨cell漏检
            double segLen=std::sqrt(dxb*dxb+dyb*dyb+dzb*dzb);
            double step=std::max(sphereR, cellSize*0.5);
            int nSamples=std::max(2, static_cast<int>(std::ceil(segLen/step)));
            double invN=1.0/static_cast<double>(nSamples-1);
            for (int si=0; si<nSamples; ++si) {
                double t=si*invN;
                queryCell(a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t);
            }
        }
    }
};

// ── Vector math (v7.3: force-inline 热路径) ──
__forceinline Vec3 ReflectDir(const Vec3& inc, const Vec3& n) {
    double d=Dot(inc,n);
    return MakeVec3(inc.x-2.0*d*n.x, inc.y-2.0*d*n.y, inc.z-2.0*d*n.z);
}
__forceinline Vec3 RefractDir(const Vec3& inc, const Vec3& n, double cosI, double n2) {
    double sin2t=(1.0-cosI*cosI)/(n2*n2);
    if (sin2t>=1.0) return ReflectDir(inc,n);
    double cosT=std::sqrt(1.0-sin2t), idn=-cosI;
    return Normalize(MakeVec3(
        (inc.x-idn*n.x)/n2 - n.x*cosT,
        (inc.y-idn*n.y)/n2 - n.y*cosT,
        (inc.z-idn*n.z)/n2 - n.z*cosT));
}

// ── Fresnel with cache (v7 H4: thread_local 消除 OpenMP 数据竞争) ──
// 分档: 16 材质 × 20 cosI 档 = 320 条目/线程
static constexpr int FCACHE_BINS = 20;

double FresnelPowerReflectionCached(double cosI, double epsR, double sigma, double freqHz,
                                     int matHash) {
    thread_local double tlCache[32][FCACHE_BINS] = {}; // 每线程独立缓存, 无竞争
    int bin = std::min(FCACHE_BINS-1, static_cast<int>(cosI * FCACHE_BINS));
    int mh = matHash & 31;
    double& cached = tlCache[mh][bin];
    if (cached > 0.0) return cached;
    // 实际计算
    double omega=6.28318530717958647693*freqHz;
    double ei=(omega>0.0)?sigma/(omega*kEpsilon0):0.0;
    Complex epsC(epsR,-ei);
    Complex s2i(1.0-cosI*cosI,0.0);
    Complex st=Sqrt(Complex(epsC.re,epsC.im)-s2i);
    Complex cic(cosI,0.0);
    Complex ec(epsC.re*cosI,epsC.im*cosI);
    cached = ((cic-st)/(cic+st)).NormSq()*0.5 + ((ec-st)/(ec+st)).NormSq()*0.5;
    return cached;
}

// ── 点到线段距离 ──
double PointToSegmentDistance(const Point3& p, const Point3& a, const Point3& b) {
    double abx=b.x-a.x, aby=b.y-a.y, abz=b.z-a.z;
    double apx=p.x-a.x, apy=p.y-a.y, apz=p.z-a.z;
    double ab2=abx*abx+aby*aby+abz*abz;
    if (ab2<=0.0) return std::sqrt(apx*apx+apy*apy+apz*apz);
    double t=(apx*abx+apy*aby+apz*abz)/ab2;
    if (t<=0.0) return std::sqrt(apx*apx+apy*apy+apz*apz);
    if (t>=1.0) { double dx=p.x-b.x,dy=p.y-b.y,dz=p.z-b.z; return std::sqrt(dx*dx+dy*dy+dz*dz); }
    double cx=a.x+t*abx, cy=a.y+t*aby, cz=a.z+t*abz;
    double dx=p.x-cx, dy=p.y-cy, dz=p.z-cz;
    return std::sqrt(dx*dx+dy*dy+dz*dz);
}

// ── 距离排序楔边查询 (RunCoarsePass 使用) ──
std::vector<int> FindNearbyWedges(const Point3& hp, const Scene* scene,
                                   double maxDist, int maxSample, int seed) {
    const auto& recs=scene->acceleration.wedge_acceleration.wedge_query_records;
    int nW=static_cast<int>(recs.size());
    if (nW==0) return {};
    double md2=maxDist*maxDist;
    int nSample=std::min(maxSample*2,nW);
    std::vector<std::pair<double,int>> cand;
    for (int i=0;i<nSample;++i) {
        int wi=(seed*17+i*31)%nW;
        const auto& w=recs[wi];
        double dx=w.center_point.x-hp.x,dy=w.center_point.y-hp.y,dz=w.center_point.z-hp.z;
        double d2=dx*dx+dy*dy+dz*dz;
        if (d2<md2) cand.push_back({d2,w.wedge_id});
    }
    std::sort(cand.begin(),cand.end());
    std::vector<int> res;
    for (int i=0;i<std::min(maxSample,static_cast<int>(cand.size()));++i)
        res.push_back(cand[i].second);
    return res;
}

// ── 重心坐标边缘检测: 检查射线命中点是否靠近面元三条边之一 ──
// 返回 true 表示命中点靠近某条边, 填充 edge_id 和 edge 方向
// 阈值 eps = λ/10 (2.4GHz → λ≈0.125m → eps≈0.0125m)
bool DetectEdgeHit(const Point3& hitPos, const Face& face, const Scene* scene,
                   double eps, int& out_edge_id, Vec3& out_edge_dir, Point3& out_edge_center) {
    const Point3& v0 = scene->vertices[face.vertex_index0];
    const Point3& v1 = scene->vertices[face.vertex_index1];
    const Point3& v2 = scene->vertices[face.vertex_index2];

    double d0 = PointToSegmentDistance(hitPos, v1, v2);
    double d1 = PointToSegmentDistance(hitPos, v2, v0);
    double d2 = PointToSegmentDistance(hitPos, v0, v1);

    int best = -1; double bestD = eps;
    if (d0 < bestD) { bestD = d0; best = 0; }
    if (d1 < bestD) { bestD = d1; best = 1; }
    if (d2 < bestD) { bestD = d2; best = 2; }

    if (best >= 0) {
        int edgeIds[3] = {face.adjacent_edge_id0, face.adjacent_edge_id1, face.adjacent_edge_id2};
        int eid = edgeIds[best];
        if (eid >= 0 && eid < static_cast<int>(scene->edges.size())) {
            const Edge& edge = scene->edges[eid];
            if (edge.supports_wedge) {
                out_edge_id = eid;
                out_edge_dir = Normalize(edge.direction);
                out_edge_center = edge.midpoint;
                return true;
            }
        }
    }
    return false;
}

} // namespace

// ── v8 GPU Wavefront: per-ray state persisted across depth levels ──

struct alignas(64) SbrRayState {
    double ox, oy, oz;
    double dx, dy, dz;
    double curPwr;
    int cr, ct, cd;
    int noNewHit;
    uint32_t rng;
    int rayIdx;
    bool alive;
    int diff_paths_stored;     // per-bounce diff path cap counter
    char _pad[7];
};
static_assert(sizeof(SbrRayState) <= 128, "SbrRayState too large");

SbrCoverageResult SbrEngine::Run(const SbrContext& context) const
{
    // v8 GPU: use megakernel or wavefront if GPU backend is configured
    if (context.config->acceleration.backend == "GPU_OptiX") {
        if (context.scene_query && context.config->sbr.max_diffraction_count == 0) {
            return RunMegakernel(context);  // no-diffraction: megakernel is fastest
        }
        return RunWavefront(context);  // diffraction enabled: use wavefront
    }
    // Legacy: per-ray serial processing (CPU-optimized)
    SbrCoverageResult result;
    if (!context.config||!context.scene||!context.scene_query) {
        result.trace_lines.push_back("SbrEngine: context incomplete"); return result;
    }
    const auto& cfg=context.config->sbr;
    const int N=cfg.ray_count, maxDepth=cfg.max_ray_depth;
    const int maxRefl=cfg.max_reflection_count;
    const int maxTrans=cfg.max_transmission_count; // v7.5: count=0即关闭
    const int maxDiff=cfg.max_diffraction_count;
    const double pwrTh=std::pow(10.0, cfg.ray_power_threshold_dB/10.0), sphereR=cfg.rx_sphere_radius_m;
    const int NRx=static_cast<int>(context.rx_grid.size());
    const double txPowerMW=std::pow(10.0, context.tx_power_dBm/10.0);
    const double normFactor=1.0/static_cast<double>(N);
    const double wedgeMD=cfg.wedge_max_distance_m;
    const int wedgeMS=cfg.wedge_max_candidates;
    const double pwrTh10=pwrTh*10.0;                     // 绕射功率阈值 (循环外预计算)
    const double originIgnoreDist=context.config->numeric_tolerance.self_hit_ignore_distance;
    // Miss segment length: Rx grid diagonal + margin, avoids Rx check blowup (was 1e6)
    double miss_seg_len = 25.0;
    if (!context.rx_grid.empty()) {
        double rxlx=context.rx_grid[0].x, rxly=context.rx_grid[0].y, rxlz=context.rx_grid[0].z;
        double rxux=rxlx, rxuy=rxly, rxuz=rxlz;
        for (auto& p : context.rx_grid) {
            if(p.x<rxlx)rxlx=p.x; if(p.x>rxux)rxux=p.x;
            if(p.y<rxly)rxly=p.y; if(p.y>rxuy)rxuy=p.y;
            if(p.z<rxlz)rxlz=p.z; if(p.z>rxuz)rxuz=p.z;
        }
        double dx=rxux-rxlx, dy=rxuy-rxly, dz=rxuz-rxlz;
        miss_seg_len = std::sqrt(dx*dx+dy*dy+dz*dz) * 2.0 + 10.0;
    }

    RxHashGrid rxGrid; rxGrid.Build(context.rx_grid,sphereR);
    result.rx_records.resize(NRx);
    for (int i=0;i<NRx;++i){result.rx_records[i].rx_position=context.rx_grid[i];result.rx_records[i].rx_index=i;}
    result.total_rays=N;
    auto rayDirs=GenerateFibonacciRays(N);

    // v7.3 W2: 射线按球面Morton码排序 → 方向相近的射线连续处理 → BVH缓存友好
    std::vector<int> rayOrder(N);
    for(int i=0;i<N;++i) rayOrder[i]=i;
    // Morton: 将单位向量(x,y,z)∈[-1,1]量化为10bit → 交织为30bit码
    auto morton3D=[&](const Vec3& d)->uint32_t{
        auto q=[&](float v){return (uint32_t)((v+1.0f)*511.5f)&0x3FF;}; // 0..1023
        uint32_t x=q((float)d.x), y=q((float)d.y), z=q((float)d.z);
        auto spread=[&](uint32_t v){v=(v|(v<<16))&0x030000FF;v=(v|(v<<8))&0x0300F00F;v=(v|(v<<4))&0x030C30C3;v=(v|(v<<2))&0x09249249;return v;};
        return spread(x)|(spread(y)<<1)|(spread(z)<<2);
    };
    std::sort(rayOrder.begin(),rayOrder.end(),[&](int a,int b){return morton3D(rayDirs[a])<morton3D(rayDirs[b]);});
    // 注: normFactor=1/N均匀, 射线权重不随方向变化, 排序无需权重补偿

    std::ostringstream oss;
#ifdef _OPENMP
    const int nTh=omp_get_max_threads();
#else
    const int nTh=1;
#endif
    oss<<"SbrEngine: rays="<<N<<" rxCount="<<NRx<<" sphereR="<<sphereR
       <<"m threads="<<nTh<<" trans="<<(maxTrans>0?"on":"off")
       <<" diff="<<(maxDiff>0?"on":"off");
    result.trace_lines.push_back(oss.str());

    std::vector<std::vector<double>> tp(nTh,std::vector<double>(NRx,0.0));
    std::vector<std::vector<int>> th(nTh,std::vector<int>(NRx,0));

    const auto* matDb=context.material_db;
    const double freqHz=context.config->em_solver.frequency_hz;
    const bool hasMatGlobal = (matDb && !matDb->empty());  // v7.3: 循环外提

    std::atomic<int> rayDone{0};
    const int reportStep = std::max(1, N / 100);
    // v8: Per-Rx 线程级路径签名去重
    std::vector<std::unordered_map<int, std::unordered_set<uint64_t>>> threadPerRxSeen(nTh);

#ifdef _OPENMP
#pragma omp parallel for schedule(static) num_threads(nTh)
#endif
    for (int ri=0; ri<N; ++ri)
    {
        int tid=0;
#ifdef _OPENMP
        tid=omp_get_thread_num();
#endif
        // v7: 进度反馈 — 射线计数器达到里程碑时输出进度 (任意线程)
        int done = ++rayDone;
        if (done % reportStep == 0 || done == N) {
#pragma omp critical
            { std::fprintf(stderr, "\r  SBR rays: %.0f%% (%d/%d)", 100.0*done/N, done, N); std::fflush(stderr); }
        }
        auto& myP=tp[tid]; auto& myH=th[tid];
        int rIdx=rayOrder[ri]; // v7.3 W2: 按Morton排序后的射线索引
        uint32_t rng=static_cast<uint32_t>(rIdx*2654435761u+1);

        Point3 curPt=context.tx_point;
        Vec3 curDir=rayDirs[rIdx];
        double curPwr=1.0;
        int cr=0, ct=0, cd=0, stepIdx=0, noNewHit=0, diffStored=0;
        std::vector<int> hitList;
        // v8: 路径节点追踪 (SBR→Precise EM)
        std::vector<PathNode> rayNodes;
        PathNode txNode; txNode.interaction_type=InteractionType::Tx; txNode.point=curPt;
        txNode.direction=curDir; txNode.valid=true;
        rayNodes.push_back(txNode);

        while (cr+ct+cd<=maxDepth && curPwr>pwrTh)
        {
            stepIdx++;
            Ray ray; ray.origin=curPt; ray.direction=curDir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance=originIgnoreDist;
            FaceHit hit=context.scene_query->QueryClosestFaceHitFast(ray,fqc);

            Point3 segEnd=hit.hit?hit.position:Add(curPt,Scale(curDir,miss_seg_len));
            std::vector<int> sHits; rxGrid.CheckSegment(curPt,segEnd,sHits);
            int newHits=0;
            for (int rxi:sHits){bool dup=false;for(int h:hitList){if(h==rxi){dup=true;break;}}if(!dup){hitList.push_back(rxi);myP[rxi]+=curPwr*normFactor;myH[rxi]++;newHits++;
                // v8: SBR路径记录+去重 — 构建GeometricPath供Precise EM使用
                if (context.store_paths) {
                    GeometricPath gp; gp.is_los=(cr+ct+cd==0);
                    gp.contains_transmission=(ct>0); gp.nodes=rayNodes;
                    gp.total_length=0; for(size_t ni=1;ni<rayNodes.size();++ni) gp.total_length+=rayNodes[ni].segment_length_from_previous;
                    PathNode rxNode; rxNode.interaction_type=InteractionType::Rx;
                    rxNode.point=context.rx_grid[rxi];
                    rxNode.direction=curDir;
                    rxNode.segment_length_from_previous=Length(Subtract(context.rx_grid[rxi],curPt));
                    rxNode.valid=true; gp.nodes.push_back(rxNode);
                    gp.total_length+=rxNode.segment_length_from_previous;
                    gp.valid=true;
                    gp.path_signature = BuildPathSignature(gp, *context.config);
#pragma omp critical
                    {
                        auto& seen = threadPerRxSeen[tid][rxi];
                        if (seen.insert(gp.path_signature).second) {
                            result.rx_records[rxi].paths.push_back(gp);
                        }
                    }
                }
            }}
            // v7.3 S1: 连续2步无新Rx命中 → 自适应终止
            if (newHits==0) { noNewHit++; if (noNewHit>=2) break; } else noNewHit=0;

            if (!hit.hit) break;
            const Face& face=context.scene->faces[hit.face_id];

            // ── Diffraction: barycentric edge detection + Keller cone sampling ──
            if (maxDiff>0 && cd<maxDiff && face.reflection_enabled
                && curPwr>pwrTh10 && !context.scene->wedges.empty()) {
                int edgeId=-1; Vec3 eD; Point3 eCenter;
                double eps = (kC0/freqHz) * 0.1; // lambda/10 threshold
                if (eps < 0.001) eps = 0.001;
                if (eps > 0.05)  eps = 0.05;
                if (DetectEdgeHit(hit.position, face, context.scene, eps, edgeId, eD, eCenter)) {
                    double cB0=std::fabs(Dot(curDir,eD)),b0=std::acos(std::min(1.0,cB0));
                    Vec3 pB=Normalize(Cross(curDir,eD)); if (Length(pB)<1e-9) goto diff_done;
                    double s1=Length(Subtract(eCenter,hit.position)); if(s1<0.1)s1=0.1;
                    double sB0=std::sqrt(1.0-cB0*cB0); if(sB0<0.05)sB0=0.05;
                    // Use generic UTD n=1.5 if wedge data unavailable; Edge data is sufficient for Keller cone
                    double k=6.28318530717958647693*freqHz/kC0;
                    double bF=4.0/k; // simplified UTD scalar coefficient
                    for (int d=0;d<4;++d){double phi=(d*0.5+0.25)*kPi;
                        Vec3 crossTerm = Cross(eD, pB);
                        Vec3 dd=MakeVec3(
                            std::sin(b0)*std::cos(phi)*pB.x+std::cos(b0)*eD.x+std::sin(b0)*std::sin(phi)*crossTerm.x,
                            std::sin(b0)*std::cos(phi)*pB.y+std::cos(b0)*eD.y+std::sin(b0)*std::sin(phi)*crossTerm.y,
                            std::sin(b0)*std::cos(phi)*pB.z+std::cos(b0)*eD.z+std::sin(b0)*std::sin(phi)*crossTerm.z);
                        Ray dr; dr.origin=eCenter; dr.direction=dd;
                        FaceHit dh=context.scene_query->QueryClosestFaceHitFast(dr,fqc);
                        Point3 dEnd=dh.hit?dh.position:Add(eCenter,Scale(dd,10.0));
                        std::vector<int> dH; rxGrid.CheckSegment(eCenter,dEnd,dH);
                        for (int drxi:dH){bool dup=false;for(int h:hitList){if(h==drxi){dup=true;break;}}
                            if(!dup){hitList.push_back(drxi);
                                double s2=Length(Subtract(context.rx_grid[drxi],eCenter));if(s2<0.1)s2=0.1;
                                myP[drxi]+=curPwr*bF*(1.0/(s1*s2*(s1+s2)))*normFactor;myH[drxi]++;
                                if (context.store_paths && diffStored < 4) {
                                    diffStored++;
                                    GeometricPath gp; gp.is_los=false; gp.contains_transmission=false;
                                    std::vector<PathNode> diffNodes=rayNodes;
                                    PathNode pn; pn.interaction_type=InteractionType::Diffraction;
                                    pn.wedge_id=edgeId; pn.point=eCenter; pn.direction=dd;
                                    pn.incident_direction=curDir; pn.surface_normal=eD;
                                    pn.segment_length_from_previous=s1; pn.valid=true;
                                    diffNodes.push_back(pn);
                                    gp.nodes=diffNodes; gp.total_length=0;
                                    for(size_t ni=1;ni<diffNodes.size();++ni) gp.total_length+=diffNodes[ni].segment_length_from_previous;
                                    PathNode rxNode; rxNode.interaction_type=InteractionType::Rx;
                                    rxNode.point=context.rx_grid[drxi]; rxNode.direction=dd;
                                    rxNode.segment_length_from_previous=s2; rxNode.valid=true;
                                    gp.nodes.push_back(rxNode); gp.total_length+=s2; gp.valid=true;
                                    gp.path_signature=BuildPathSignature(gp,*context.config);
                                    auto& seen = threadPerRxSeen[tid][drxi];
                                    if (seen.insert(gp.path_signature).second) result.rx_records[drxi].paths.push_back(gp);
                                }
                            }}
                    }
                }
                diff_done:;
            }

            // ── 材质 (v7.3: 循环外提hasMatGlobal + const ref) ──
            double cosI=std::fabs(Dot(curDir,hit.normal)); if(cosI<1e-4)cosI=1e-4;
            double refPwr=0.3; double transEpsR=1.0; int matHash=0; bool hasMat=false;
            if (hasMatGlobal) {
                matHash=static_cast<int>(face.surface_eps_r*100)+static_cast<int>(face.surface_sigma*1000);
                refPwr=FresnelPowerReflectionCached(cosI,face.surface_eps_r,face.surface_sigma,freqHz,matHash);
                transEpsR=face.surface_eps_r; hasMat=true;
            }

            // ── 透射 (Monte Carlo: 概率选择+物理功率衰减, 不分裂) ──
            // v9 step8: 预检TIR — 若发生全内反射则不尝试透射
            if (maxTrans>0 && face.transmission_enabled && ct<maxTrans && hasMat
                && face.dual_side_material_resolved) {
                double r=RandDouble(rng);
                Vec3 oldDir=curDir; Point3 oldPt=curPt; // save for PathNode
                if (r>=refPwr) { // 透射 (概率=1-|Γ|²)
                    double n2=std::sqrt(std::max(1.0,transEpsR));
                    double sinT2 = (1.0 - cosI*cosI) / (n2*n2);
                    // v9 step8: TIR预检 — 全内反射时退化为反射而非错误记录透射
                    if (sinT2 >= 1.0) {
                        // TIR: 能量全反射, 走下面反射分支
                        curPwr*=refPwr; cr++;
                        curDir=ReflectDir(curDir,hit.normal);
                        curPt=Add(hit.position,Scale(curDir,0.01));
                        if(context.store_paths){PathNode pn;pn.interaction_type=InteractionType::Reflection;
                            pn.face_id=hit.face_id;pn.point=hit.position;pn.direction=curDir;
                            pn.incident_direction=oldDir;pn.surface_normal=hit.normal;
                            pn.segment_length_from_previous=Length(Subtract(hit.position,oldPt));pn.valid=true;
                            rayNodes.push_back(pn);}
                        continue;
                    }
                    curDir=RefractDir(curDir,hit.normal,cosI,n2);
                    curPt=Add(hit.position,Scale(curDir,0.01));
                    curPwr*=(1.0-refPwr); ct++;
                    if(context.store_paths){PathNode pn;pn.interaction_type=InteractionType::Transmission;
                        pn.face_id=hit.face_id;pn.point=hit.position;pn.direction=curDir;
                        pn.incident_direction=oldDir;pn.surface_normal=hit.normal;
                        pn.segment_length_from_previous=Length(Subtract(hit.position,oldPt));pn.valid=true;
                        rayNodes.push_back(pn);}
                    continue; // 物理透射衰减
                }
                // 反射 (概率=|Γ|²) — 走下面反射分支 (curPwr*=refPwr)
            }

            // ── 反射 ──
            if (face.reflection_enabled && cr<maxRefl) {
                Vec3 oldDir=curDir; Point3 oldPt=curPt;
                curDir=ReflectDir(curDir,hit.normal);
                curPt=Add(hit.position,Scale(curDir,0.01));
                curPwr*=refPwr; cr++;
                if(context.store_paths){PathNode pn;pn.interaction_type=InteractionType::Reflection;
                    pn.face_id=hit.face_id;pn.point=hit.position;pn.direction=curDir;
                    pn.incident_direction=oldDir;pn.surface_normal=hit.normal;
                    pn.segment_length_from_previous=Length(Subtract(hit.position,oldPt));pn.valid=true;
                    rayNodes.push_back(pn);}
                continue;
            }

            break; // 非交互面终止
        }
    }

    std::fprintf(stderr, "\n"); // SBR进度收尾换行

    // 归并
    for (int t=0;t<nTh;++t){auto&tp_=tp[t];auto&th_=th[t];
        for (int i=0;i<NRx;++i){if(th_[i]>0){result.rx_records[i].total_power_linear+=tp_[i];result.rx_records[i].ray_hit_count+=th_[i];}}}

    for (auto& rec:result.rx_records){
        if(rec.ray_hit_count>0){
            rec.total_power_dBm=10.0*std::log10(std::max(1e-30,rec.total_power_linear*txPowerMW));
            result.active_rx_count++;
        } else {
            rec.total_power_dBm = -200.0; // 未命中Rx: colorbar最低值
        }
    }
    result.succeeded=true;
    oss.str("");oss<<"SbrEngine: activeRx="<<result.active_rx_count<<"/"<<NRx;
    result.trace_lines.push_back(oss.str());
    return result;
}

// ═══════════════════════════════════════════════════════════════════
//  v8 Phase 2: RunCoarsePass — 面元可见性收集 (非功率累加)
// ═══════════════════════════════════════════════════════════════════
SbrCoarseResult SbrEngine::RunCoarsePass(const SbrCoarseContext& context) const
{
    SbrCoarseResult result;
    if (!context.scene || !context.scene_query) {
        result.trace_lines.push_back("SbrCoarsePass: context incomplete");
        return result;
    }

    const int N = context.coarse_ray_count;
    const int maxDepth = context.coarse_max_depth;
    const double sphereR = context.coarse_rx_sphere_radius;
    const int NRx = static_cast<int>(context.rx_grid.size());
    const double originIgnoreDist = context.config
        ? context.config->numeric_tolerance.self_hit_ignore_distance : 1.0e-5;
    double miss_seg_len = 25.0;  // coarse miss: fixed conservative length

    // ── RxHashGrid (复用现有, 放大接收球) ──
    RxHashGrid rxGrid;
    rxGrid.Build(context.rx_grid, sphereR);

    result.total_coarse_rays = N;
    result.rx_active_mask.resize(NRx, false);
    result.rx_active_faces.clear();
    result.rx_active_wedges.clear();

    auto rayDirs = GenerateFibonacciRays(N);

    // PVS 引用 (若可用)
    const auto* pvs = (context.expand_pvs && context.scene->visibility.face_pvs.valid)
        ? &context.scene->visibility.face_pvs : nullptr;

    // Morton 排序 (复用现有)
    std::vector<int> rayOrder(N);
    for (int i = 0; i < N; ++i) rayOrder[i] = i;
    auto morton3D = [&](const Vec3& d) -> uint32_t {
        auto q = [&](float v) { return (uint32_t)((v + 1.0f) * 511.5f) & 0x3FF; };
        uint32_t x = q((float)d.x), y = q((float)d.y), z = q((float)d.z);
        auto spread = [&](uint32_t v) {
            v = (v | (v << 16)) & 0x030000FF; v = (v | (v << 8)) & 0x0300F00F;
            v = (v | (v << 4)) & 0x030C30C3; v = (v | (v << 2)) & 0x09249249; return v;
        };
        return spread(x) | (spread(y) << 1) | (spread(z) << 2);
    };
    std::sort(rayOrder.begin(), rayOrder.end(),
        [&](int a, int b) { return morton3D(rayDirs[a]) < morton3D(rayDirs[b]); });

    std::ostringstream oss;
    oss << "SbrCoarsePass: rays=" << N << " rxCount=" << NRx
        << " sphereR=" << sphereR << "m maxDepth=" << maxDepth
        << " PVS=" << (pvs ? "on" : "off");
    result.trace_lines.push_back(oss.str());

#pragma omp parallel for schedule(dynamic)
    for (int ri = 0; ri < N; ++ri)
    {
        Point3 curPt = context.tx_point;
        Vec3 curDir = rayDirs[rayOrder[ri]];
        int depth = 0;
        int ignoredFace = -1;

        while (depth <= maxDepth)
        {
            Ray ray; ray.origin = curPt; ray.direction = curDir;
            FaceQueryContext fqc;
            fqc.ignored_face_id = ignoredFace;
            fqc.origin_ignore_distance = originIgnoreDist;
            FaceHit hit = context.scene_query->QueryClosestFaceHitFast(ray, fqc);

            Point3 segEnd = hit.hit ? hit.position : Add(curPt, Scale(curDir, miss_seg_len));

            // ── Rx 命中: 收集面元可见性 ──
            std::vector<int> sHits;
            rxGrid.CheckSegment(curPt, segEnd, sHits);
            for (int rxi : sHits)
            {
#pragma omp critical
                {
                    result.rx_active_mask[rxi] = true;
                    auto& faces = result.rx_active_faces[rxi];
                    auto& wedges = result.rx_active_wedges[rxi];

                    if (hit.hit) {
                        // 记录命中面元
                        if (std::find(faces.begin(), faces.end(), hit.face_id) == faces.end())
                            faces.push_back(hit.face_id);
                        // PVS 扩展: 加入该面元的可见面元
                        if (pvs && pvs->HasEntry(hit.face_id)) {
                            for (int pvsFace : pvs->GetVisibleFaces(hit.face_id)) {
                                if (std::find(faces.begin(), faces.end(), pvsFace) == faces.end())
                                    faces.push_back(pvsFace);
                            }
                        }
                    }

                    // 近邻楔边收集
                    if (!context.scene->wedges.empty() && hit.hit) {
                        double wedgeMD = 5.0;
                        int wedgeMS = 16;
                        uint32_t seed = static_cast<uint32_t>(ri * 2654435761u + depth);
                        auto nw = FindNearbyWedges(hit.position, context.scene, wedgeMD, wedgeMS, seed);
                        for (int wid : nw) {
                            if (std::find(wedges.begin(), wedges.end(), wid) == wedges.end())
                                wedges.push_back(wid);
                        }
                    }
                }
            }

            if (!hit.hit) break;
            const Face& face = context.scene->faces[hit.face_id];

            // 仅反射 (粗扫不处理透射/绕射)
            if (face.reflection_enabled) {
                curDir = ReflectDir(curDir, hit.normal);
                curPt = Add(hit.position, Scale(curDir, 0.01));
                ignoredFace = hit.face_id;
                depth++;
            } else {
                break;
            }
        }
    }

    // 统计活跃 Rx
    for (bool active : result.rx_active_mask)
        if (active) result.active_rx_count++;

    result.succeeded = (result.active_rx_count > 0);
    oss.str("");
    oss << "SbrCoarsePass: activeRx=" << result.active_rx_count << "/" << NRx
        << " (" << (100.0 * result.active_rx_count / std::max(1, NRx)) << "%)";
    result.trace_lines.push_back(oss.str());
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════
//  v8 GPU: Wavefront batch ray processing
//  按深度分层: 每层所有活跃射线一次性批量查询 GPU,
//  然后 CPU 处理命中结果并生成下一层射线
// ═══════════════════════════════════════════════════════════════════════════
SbrCoverageResult SbrEngine::RunWavefront(const SbrContext& context) const
{
    SbrCoverageResult result;
    const auto& cfg = context.config->sbr;
    const int N = cfg.ray_count;
    const int maxDepth = cfg.max_ray_depth;
    const int maxRefl = cfg.max_reflection_count;
    const int maxTrans = cfg.max_transmission_count;
    const int maxDiff = cfg.max_diffraction_count;
    const double pwrTh = std::pow(10.0, cfg.ray_power_threshold_dB / 10.0);
    const double sphereR = cfg.rx_sphere_radius_m;
    const int NRx = static_cast<int>(context.rx_grid.size());
    const double txPowerMW = std::pow(10.0, context.tx_power_dBm / 10.0);
    const double normFactor = 1.0 / static_cast<double>(N);
    const double wedgeMD = cfg.wedge_max_distance_m;
    const int wedgeMS = cfg.wedge_max_candidates;
    const double pwrTh10 = pwrTh * 10.0;
    const double originIgnoreDist = context.config->numeric_tolerance.self_hit_ignore_distance;
    const double freqHz = context.config->em_solver.frequency_hz;
    const auto* matDb = context.material_db;
    const bool hasMatGlobal = (matDb && !matDb->empty());

    // Miss segment length from Rx grid bounds
    double wf_miss_seg_len = 25.0;
    if (!context.rx_grid.empty()) {
        double rxlx=context.rx_grid[0].x, rxly=context.rx_grid[0].y, rxlz=context.rx_grid[0].z;
        double rxux=rxlx, rxuy=rxly, rxuz=rxlz;
        for (auto& p : context.rx_grid) {
            if(p.x<rxlx)rxlx=p.x; if(p.x>rxux)rxux=p.x;
            if(p.y<rxly)rxly=p.y; if(p.y>rxuy)rxuy=p.y;
            if(p.z<rxlz)rxlz=p.z; if(p.z>rxuz)rxuz=p.z;
        }
        double dx=rxux-rxlx, dy=rxuy-rxly, dz=rxuz-rxlz;
        wf_miss_seg_len = std::sqrt(dx*dx+dy*dy+dz*dz) * 2.0 + 10.0;
    }

    // ── Rx spatial grid ──
    RxHashGrid rxGrid;
    rxGrid.Build(context.rx_grid, sphereR);
    result.rx_records.resize(NRx);
    for (int i = 0; i < NRx; ++i) {
        result.rx_records[i].rx_position = context.rx_grid[i];
        result.rx_records[i].rx_index = i;
    }
    result.total_rays = N;

    // ── Fibonacci ray directions + Morton sort ──
    auto rayDirs = GenerateFibonacciRays(N);
    std::vector<int> rayOrder(N);
    for (int i = 0; i < N; ++i) rayOrder[i] = i;
    auto morton3D = [&](const Vec3& d) -> uint32_t {
        auto q = [&](float v) { return (uint32_t)((v + 1.0f) * 511.5f) & 0x3FF; };
        uint32_t x = q((float)d.x), y = q((float)d.y), z = q((float)d.z);
        auto spread = [&](uint32_t v) {
            v = (v | (v << 16)) & 0x030000FF; v = (v | (v << 8)) & 0x0300F00F;
            v = (v | (v << 4)) & 0x030C30C3; v = (v | (v << 2)) & 0x09249249; return v;
        };
        return spread(x) | (spread(y) << 1) | (spread(z) << 2);
    };
    std::sort(rayOrder.begin(), rayOrder.end(),
        [&](int a, int b) { return morton3D(rayDirs[a]) < morton3D(rayDirs[b]); });

    // ── Per-thread power accumulators (same as legacy) ──
#ifdef _OPENMP
    const int nTh = omp_get_max_threads();
#else
    const int nTh = 1;
#endif
    std::vector<std::vector<double>> tp(nTh, std::vector<double>(NRx, 0.0));
    std::vector<std::vector<int>> th(nTh, std::vector<int>(NRx, 0));

    // ── Per-Rx path signature dedup ──
    std::vector<std::unordered_map<int, std::unordered_set<uint64_t>>> threadPerRxSeen(nTh);

    // ── Wave 1: all rays from Tx ──
    std::vector<SbrRayState> wave; wave.reserve(N);
    for (int ri = 0; ri < N; ++ri) {
        int rIdx = rayOrder[ri];
        SbrRayState s;
        s.ox = context.tx_point.x; s.oy = context.tx_point.y; s.oz = context.tx_point.z;
        const Vec3& d = rayDirs[rIdx];
        s.dx = d.x; s.dy = d.y; s.dz = d.z;
        s.curPwr = 1.0;
        s.cr = 0; s.ct = 0; s.cd = 0;
        s.noNewHit = 0;
        s.rng = static_cast<uint32_t>(rIdx * 2654435761u + 1);
        s.rayIdx = rIdx;
        s.alive = true;
        s.diff_paths_stored = 0;
        wave.push_back(s);
    }

    // ── Hit list per ray (external: 大部分射线终止很早, 不每个分配) ──
    // 使用 compact 存储: 95% 的射线在 depth 2 前终止, 仅活跃射线需要 hitList
    // 这里用 deferred allocation: 仅在需要时分配
    struct RayAux {
        std::vector<int> hitList;
        std::vector<PathNode> rayNodes;
        int total_paths = 0;    // hard cap: max 8 paths per ray total
    };
    std::vector<RayAux> aux(wave.size());

    // Init rayNodes for all rays
    for (size_t i = 0; i < wave.size(); ++i) {
        if (context.store_paths) {
            PathNode txNode;
            txNode.interaction_type = InteractionType::Tx;
            txNode.point = MakeVec3(wave[i].ox, wave[i].oy, wave[i].oz);
            txNode.direction = MakeVec3(wave[i].dx, wave[i].dy, wave[i].dz);
            txNode.valid = true;
            aux[i].rayNodes.push_back(txNode);
        }
    }

    std::ostringstream oss;
    oss << "SbrEngine[wavefront]: rays=" << N << " rxCount=" << NRx
        << " sphereR=" << sphereR << "m threads=" << nTh
        << " trans=" << (maxTrans > 0 ? "on" : "off")
        << " diff=" << (maxDiff > 0 ? "on" : "off");
    result.trace_lines.push_back(oss.str());

    // ── Depth-by-depth wavefront loop ──
    for (int depth = 0; depth <= maxDepth; ++depth)
    {
        // Compact: collect alive rays
        std::vector<int> aliveIdx;
        std::vector<Ray> batchRays;
        for (size_t i = 0; i < wave.size(); ++i) {
            if (wave[i].alive) {
                aliveIdx.push_back(static_cast<int>(i));
                Ray r;
                r.origin = MakeVec3(wave[i].ox, wave[i].oy, wave[i].oz);
                r.direction = MakeVec3(wave[i].dx, wave[i].dy, wave[i].dz);
                batchRays.push_back(r);
            }
        }

        int nAlive = static_cast<int>(aliveIdx.size());
        if (nAlive == 0) break;

        std::fprintf(stderr, "\r  SBR wavefront: depth=%d alive=%d/%d",
            depth, nAlive, N);
        std::fflush(stderr);

        // ── Batch GPU BVH query ──
        FaceQueryContext fqc;
        fqc.origin_ignore_distance = originIgnoreDist;
        std::vector<FaceHit> batchHits = context.scene_query->QueryClosestFaceHitBatch(batchRays, fqc);

        // ── Diffraction: collect rays for batch GPU processing ──
        struct DiffRayMeta {
            int parentIdx;
            int wedgeId;
            double s1, bF;
            double dd_x, dd_y, dd_z;
            double cB0;
            double eD_x, eD_y, eD_z;
        };
        std::vector<Ray> diffRays;
        std::vector<double> diffStarts, diffEnds;
        std::vector<DiffRayMeta> diffMeta;

        if (maxDiff > 0 && !context.scene->wedges.empty()) {
            // Pre-allocate: max 200K rays × 8 wedges × 4 directions = 6.4M
            size_t diffEstimate = static_cast<size_t>(nAlive) * 8 * 4;
            diffRays.reserve(diffEstimate);
            diffStarts.reserve(diffEstimate * 3);
            diffEnds.reserve(diffEstimate * 3);
            diffMeta.reserve(diffEstimate);
            for (int bi = 0; bi < nAlive; ++bi) {
                size_t wi = aliveIdx[bi];
                const FaceHit& hit = batchHits[bi];
                if (!hit.hit) continue;
                const Face& face = context.scene->faces[hit.face_id];
                if (!face.reflection_enabled) continue;
                if (wave[wi].cd >= maxDiff) continue;
                if (wave[wi].curPwr <= pwrTh10) continue;

                Vec3 curDir = MakeVec3(wave[wi].dx, wave[wi].dy, wave[wi].dz);
                auto nw = FindNearbyWedges(hit.position, context.scene, wedgeMD, wedgeMS, wi + depth);
                for (int wid : nw) {
                    const Wedge& w = context.scene->wedges[wid];
                    Vec3 eD = Normalize(w.direction);
                    if (Length(eD) < 1e-9) continue;
                    double cB0 = std::fabs(Dot(curDir, eD));
                    Vec3 pB = Normalize(Cross(curDir, eD));
                    if (Length(pB) < 1e-9) continue;
                    double s1 = Length(Subtract(w.center_point, hit.position)); if (s1 < 0.1) s1 = 0.1;
                    double sB0 = std::sqrt(1.0 - cB0 * cB0); if (sB0 < 0.05) sB0 = 0.05;
                    double nW = (360.0 - w.wedge_angle_deg) / 180.0; if (nW < 0.5) nW = 0.5;
                    double k = 6.28318530717958647693 * freqHz / kC0;
                    double bF = 8.0 / k * (1.0 / (nW * nW)) * (1.0 / (sB0 * sB0));
                    double b0 = std::acos(std::min(1.0, cB0));
                    for (int d = 0; d < 4; ++d) {
                        double phi = (d * 0.5 + 0.25) * kPi;
                        Vec3 crossTerm = Cross(eD, pB);
                        Vec3 dd = MakeVec3(
                            std::sin(b0) * std::cos(phi) * pB.x + std::cos(b0) * eD.x + std::sin(b0) * std::sin(phi) * crossTerm.x,
                            std::sin(b0) * std::cos(phi) * pB.y + std::cos(b0) * eD.y + std::sin(b0) * std::sin(phi) * crossTerm.y,
                            std::sin(b0) * std::cos(phi) * pB.z + std::cos(b0) * eD.z + std::sin(b0) * std::sin(phi) * crossTerm.z);
                        Ray dr; dr.origin = w.center_point; dr.direction = dd;
                        diffRays.push_back(dr);
                        diffStarts.push_back(w.center_point.x);
                        diffStarts.push_back(w.center_point.y);
                        diffStarts.push_back(w.center_point.z);
                        diffEnds.push_back(w.center_point.x + dd.x * 50.0); // long segment for Rx check
                        diffEnds.push_back(w.center_point.y + dd.y * 50.0);
                        diffEnds.push_back(w.center_point.z + dd.z * 50.0);
                        DiffRayMeta m;
                        m.parentIdx = bi; m.wedgeId = wid;
                        m.s1 = s1; m.bF = bF;
                        m.dd_x = dd.x; m.dd_y = dd.y; m.dd_z = dd.z;
                        m.cB0 = cB0;
                        m.eD_x = eD.x; m.eD_y = eD.y; m.eD_z = eD.z;
                        diffMeta.push_back(m);
                    }
                }
            }
        }

        // Build diff ray index: diffOffset[bi] = start index in diffMeta
        std::vector<int> diffOffset(nAlive + 1, 0);
        int diffTotal = static_cast<int>(diffMeta.size());
        for (int dmi = 0, curParent = -1; dmi < diffTotal; ++dmi) {
            if (diffMeta[dmi].parentIdx != curParent) {
                curParent = diffMeta[dmi].parentIdx;
                diffOffset[curParent] = dmi;
            }
        }
        diffOffset[nAlive] = diffTotal; // sentinel

        // GPU batch BVH for diffraction rays
        std::vector<FaceHit> diffHits;
        if (!diffRays.empty()) {
            diffHits = context.scene_query->QueryClosestFaceHitBatch(diffRays, fqc);
            // Update diffEnds to actual hit positions for Rx check
            for (size_t di = 0; di < diffHits.size(); ++di) {
                if (diffHits[di].hit) {
                    diffEnds[di * 3 + 0] = diffHits[di].position.x;
                    diffEnds[di * 3 + 1] = diffHits[di].position.y;
                    diffEnds[di * 3 + 2] = diffHits[di].position.z;
                }
            }
        }

        // GPU batch Rx query for diffraction (deferred: rxParams not yet built)
        std::vector<double> seg_starts_flat, seg_ends_flat;
        seg_starts_flat.reserve(nAlive * 3);
        seg_ends_flat.reserve(nAlive * 3);
        for (int bi = 0; bi < nAlive; ++bi) {
            size_t wi = aliveIdx[bi];
            const FaceHit& hit = batchHits[bi];
            seg_starts_flat.push_back(wave[wi].ox);
            seg_starts_flat.push_back(wave[wi].oy);
            seg_starts_flat.push_back(wave[wi].oz);
            if (hit.hit) {
                seg_ends_flat.push_back(hit.position.x);
                seg_ends_flat.push_back(hit.position.y);
                seg_ends_flat.push_back(hit.position.z);
            } else {
                seg_ends_flat.push_back(wave[wi].ox + wave[wi].dx * wf_miss_seg_len);
                seg_ends_flat.push_back(wave[wi].oy + wave[wi].dy * wf_miss_seg_len);
                seg_ends_flat.push_back(wave[wi].oz + wave[wi].dz * wf_miss_seg_len);
            }
        }

        // v9 step13: 移除static — 每次调用重新构建RxGrid, 防止跨运行污染
        std::vector<int> rxCellOffsets;
        std::vector<int> rxFlatData;
        ISceneAccelerator::RxGridQueryParams rxParams;
        bool rxGridBuilt = false;

        if (!rxGridBuilt) {
            int cellCount = rxGrid.nx * rxGrid.ny * rxGrid.nz;
            rxCellOffsets.resize(cellCount + 1, 0);
            rxFlatData.clear();
            for (int ci = 0; ci < cellCount; ++ci) {
                rxCellOffsets[ci] = static_cast<int>(rxFlatData.size());
                for (int rxIdx : rxGrid.flatCells[ci])
                    rxFlatData.push_back(rxIdx);
            }
            rxCellOffsets[cellCount] = static_cast<int>(rxFlatData.size());
            rxParams.rx_positions = context.rx_grid.data();
            rxParams.rx_count = NRx;
            rxParams.cell_size = rxGrid.cellSize;
            rxParams.sphere_radius = rxGrid.sphereR;
            rxParams.ox = rxGrid.ox; rxParams.oy = rxGrid.oy; rxParams.oz = rxGrid.oz;
            rxParams.nx = rxGrid.nx; rxParams.ny = rxGrid.ny; rxParams.nz = rxGrid.nz;
            rxParams.max_hits_per_seg = 256;
            rxParams.cell_offsets = rxCellOffsets.data();
            rxParams.flat_cell_data = rxFlatData.data();
            rxParams.flat_cell_data_size = static_cast<int>(rxFlatData.size());
            rxGridBuilt = true;
        }

        // GPU batch Rx query (main rays)
        std::vector<std::vector<int>> allRxHits;
        allRxHits = context.scene_query->QueryRxHitsBatch(seg_starts_flat, seg_ends_flat, rxParams);

        // GPU batch Rx query (diffraction — reduced max_hits: 32 vs 256 for main)
        std::vector<std::vector<int>> diffRxHits;
        if (!diffEnds.empty()) {
            ISceneAccelerator::RxGridQueryParams diffRxParams = rxParams;
            diffRxParams.max_hits_per_seg = 96;
            diffRxHits = context.scene_query->QueryRxHitsBatch(diffStarts, diffEnds, diffRxParams);
        }

        // CPU parallel processing
        std::vector<SbrRayState> nextWave;
        std::vector<RayAux> nextAux;
        nextWave.reserve(nAlive);
        nextAux.reserve(nAlive);

#pragma omp parallel for schedule(static)
        for (int bi = 0; bi < nAlive; ++bi) {
            int tid = 0;
#ifdef _OPENMP
            tid = omp_get_thread_num();
#endif
            size_t wi = aliveIdx[bi];
            if (!wave[wi].alive) continue;

            const FaceHit& hit = batchHits[bi];
            double curPwr = wave[wi].curPwr;
            int cr = wave[wi].cr, ct = wave[wi].ct, cd = wave[wi].cd;
            uint32_t rng = wave[wi].rng;
            auto& myP = tp[tid];
            auto& myH = th[tid];
            Point3 curPt = MakeVec3(wave[wi].ox, wave[wi].oy, wave[wi].oz);
            Vec3 curDir = MakeVec3(wave[wi].dx, wave[wi].dy, wave[wi].dz);

            // Rx hits from GPU batch result; dedup via unordered_set (O(1) vs O(K))
            const std::vector<int>& sHits = allRxHits[bi];
            auto& hitList = aux[wi].hitList;
            int newHits = 0;
            for (int rxi : sHits) {
                bool dup = false;
                for (int h : hitList) { if (h == rxi) { dup = true; break; } }
                if (!dup) {
                    hitList.push_back(rxi);
                    newHits++;
                    myP[rxi] += curPwr * normFactor;
                    myH[rxi]++;
                    // Total path cap: 8 per ray total (avoids OOM with 2M rays)
                    if (context.store_paths && aux[wi].total_paths < 8) {
                        aux[wi].total_paths++;
                        GeometricPath gp;
                        gp.is_los = (cr + ct + cd == 0);
                        gp.contains_transmission = (ct > 0);
                        gp.nodes = aux[wi].rayNodes;
                        gp.total_length = 0;
                        for (size_t ni = 1; ni < aux[wi].rayNodes.size(); ++ni)
                            gp.total_length += aux[wi].rayNodes[ni].segment_length_from_previous;
                        PathNode rxNode;
                        rxNode.interaction_type = InteractionType::Rx;
                        rxNode.point = context.rx_grid[rxi];
                        rxNode.direction = curDir;
                        rxNode.segment_length_from_previous = Length(Subtract(context.rx_grid[rxi], curPt));
                        rxNode.valid = true;
                        gp.nodes.push_back(rxNode);
                        gp.total_length += rxNode.segment_length_from_previous;
                        gp.valid = true;
                        gp.path_signature = BuildPathSignature(gp, *context.config);
#pragma omp critical(path_record)
                        {
                            auto& seen = threadPerRxSeen[tid][rxi];
                            if (seen.insert(gp.path_signature).second)
                                result.rx_records[rxi].paths.push_back(gp);
                        }
                    }
                }
            }

            int noNewHit = wave[wi].noNewHit;
            if (newHits == 0) { noNewHit++; if (noNewHit >= 2) { wave[wi].alive = false; continue; } }
            else noNewHit = 0;

            if (!hit.hit) { wave[wi].alive = false; continue; }
            const Face& face = context.scene->faces[hit.face_id];

            // Diffraction: barycentric edge detection + Keller cone (inline CPU, per-ray)
            if (maxDiff > 0 && cd < maxDiff && face.reflection_enabled
                && curPwr > pwrTh10 && !context.scene->wedges.empty()) {
                int edgeId=-1; Vec3 eD; Point3 eCenter;
                double eps = (kC0/freqHz) * 0.1; if (eps < 0.001) eps = 0.001; if (eps > 0.05) eps = 0.05;
                if (DetectEdgeHit(hit.position, face, context.scene, eps, edgeId, eD, eCenter)) {
                    double cB0=std::fabs(Dot(curDir,eD)),b0=std::acos(std::min(1.0,cB0));
                    Vec3 pB=Normalize(Cross(curDir,eD)); if (Length(pB)<1e-9) goto diff_done_wf;
                    double s1=Length(Subtract(eCenter,hit.position)); if(s1<0.1)s1=0.1;
                    double k=6.28318530717958647693*freqHz/kC0;
                    double bF=4.0/k;
                    for (int d=0;d<4;++d){double phi=(d*0.5+0.25)*kPi;
                        Vec3 crossTerm = Cross(eD, pB);
                        Vec3 dd=MakeVec3(
                            std::sin(b0)*std::cos(phi)*pB.x+std::cos(b0)*eD.x+std::sin(b0)*std::sin(phi)*crossTerm.x,
                            std::sin(b0)*std::cos(phi)*pB.y+std::cos(b0)*eD.y+std::sin(b0)*std::sin(phi)*crossTerm.y,
                            std::sin(b0)*std::cos(phi)*pB.z+std::cos(b0)*eD.z+std::sin(b0)*std::sin(phi)*crossTerm.z);
                        Ray dr; dr.origin=eCenter; dr.direction=dd;
                        FaceHit dh=context.scene_query->QueryClosestFaceHitFast(dr,fqc);
                        Point3 dEnd=dh.hit?dh.position:Add(eCenter,Scale(dd,10.0));
                        std::vector<int> dH; rxGrid.CheckSegment(eCenter,dEnd,dH);
                        for (int drxi:dH){bool dup=false;for(int h:hitList){if(h==drxi){dup=true;break;}}
                            if(!dup){hitList.push_back(drxi);
                                double s2=Length(Subtract(context.rx_grid[drxi],eCenter));if(s2<0.1)s2=0.1;
                                myP[drxi]+=curPwr*bF*(1.0/(s1*s2*(s1+s2)))*normFactor;myH[drxi]++;
                                if (context.store_paths && wave[wi].diff_paths_stored < 4) {
                                    wave[wi].diff_paths_stored++;
                                    GeometricPath gp; gp.is_los=false; gp.contains_transmission=false;
                                    std::vector<PathNode> diffNodes=aux[wi].rayNodes;
                                    PathNode pn; pn.interaction_type=InteractionType::Diffraction;
                                    pn.wedge_id=edgeId; pn.point=eCenter; pn.direction=dd;
                                    pn.incident_direction=curDir; pn.surface_normal=eD;
                                    pn.segment_length_from_previous=s1; pn.valid=true;
                                    diffNodes.push_back(pn);
                                    gp.nodes=diffNodes; gp.total_length=0;
                                    for(size_t ni=1;ni<diffNodes.size();++ni) gp.total_length+=diffNodes[ni].segment_length_from_previous;
                                    PathNode rxNode; rxNode.interaction_type=InteractionType::Rx;
                                    rxNode.point=context.rx_grid[drxi]; rxNode.direction=dd;
                                    rxNode.segment_length_from_previous=s2; rxNode.valid=true;
                                    gp.nodes.push_back(rxNode); gp.total_length+=s2; gp.valid=true;
                                    gp.path_signature=BuildPathSignature(gp,*context.config);
#pragma omp critical(path_record)
                                    { auto& seen=threadPerRxSeen[tid][drxi];
                                      if(seen.insert(gp.path_signature).second) result.rx_records[drxi].paths.push_back(gp); }
                                }
                            }}
                    }
                }
                diff_done_wf:;
            }

            double cosI = std::fabs(Dot(curDir, hit.normal)); if (cosI < 1e-4) cosI = 1e-4;
            double refPwr = 0.3; double transEpsR = 1.0; int matHash = 0; bool hasMat = false;
            if (hasMatGlobal) {
                matHash = static_cast<int>(face.surface_eps_r * 100) + static_cast<int>(face.surface_sigma * 1000);
                refPwr = FresnelPowerReflectionCached(cosI, face.surface_eps_r, face.surface_sigma, freqHz, matHash);
                transEpsR = face.surface_eps_r; hasMat = true;
            }

            // Transmission (Monte Carlo) — per-thread buffer, no omp critical
            // v9 step8: 预检TIR — 全内反射时退化为反射而非错误记录透射
            if (maxTrans > 0 && face.transmission_enabled && ct < maxTrans && hasMat
                && face.dual_side_material_resolved) {
                double r = RandDouble(rng);
                if (r >= refPwr) {
                    double n2 = std::sqrt(std::max(1.0, transEpsR));
                    double sinT2 = (1.0 - cosI*cosI) / (n2*n2); // v9 step8: TIR预检
                    if (sinT2 >= 1.0) {
                        // TIR: 退化为反射 — 走下面反射分支, 不产生透射状态
                        rng = rng; // suppress unused warning, fall through to reflection
                    } else {
                        Vec3 newDir = RefractDir(curDir, hit.normal, cosI, n2);
                        Point3 newPt = Add(hit.position, Scale(newDir, 0.01));
                        double newPwr = curPwr * (1.0 - refPwr);
                        SbrRayState ns;
                        ns.ox = newPt.x; ns.oy = newPt.y; ns.oz = newPt.z;
                        ns.dx = newDir.x; ns.dy = newDir.y; ns.dz = newDir.z;
                        ns.curPwr = newPwr;
                        ns.cr = cr; ns.ct = ct + 1; ns.cd = cd;
                        ns.noNewHit = noNewHit; ns.diff_paths_stored = 0;
                        ns.rng = rng; ns.rayIdx = wave[wi].rayIdx;
                        ns.alive = (ns.curPwr > pwrTh);
                        RayAux na;
                        na.hitList = aux[wi].hitList;
                        na.total_paths = aux[wi].total_paths;
                        if (context.store_paths) {
                            na.rayNodes = aux[wi].rayNodes;
                            PathNode pn; pn.interaction_type = InteractionType::Transmission;
                            pn.face_id = hit.face_id; pn.point = hit.position; pn.direction = newDir;
                            pn.incident_direction = curDir; pn.surface_normal = hit.normal;
                            pn.segment_length_from_previous = Length(Subtract(hit.position, curPt)); pn.valid = true;
                            na.rayNodes.push_back(pn);
                        }
                        #pragma omp critical(nextwave)
                        {
                            nextWave.push_back(ns);
                            nextAux.push_back(na);
                        }
                        continue;
                    } // end else (normal transmission)
                } // end if (r >= refPwr)
            } // end transmission block (v9 step8)

            // Reflection — per-thread buffer, no omp critical
            if (face.reflection_enabled && cr < maxRefl) {
                Vec3 newDir = ReflectDir(curDir, hit.normal);
                Point3 newPt = Add(hit.position, Scale(newDir, 0.01));
                double newPwr = curPwr * refPwr;
                SbrRayState ns;
                ns.ox = newPt.x; ns.oy = newPt.y; ns.oz = newPt.z;
                ns.dx = newDir.x; ns.dy = newDir.y; ns.dz = newDir.z;
                ns.curPwr = newPwr;
                ns.cr = cr + 1; ns.ct = ct; ns.cd = cd;
                ns.noNewHit = noNewHit; ns.diff_paths_stored = 0;
                ns.rng = rng; ns.rayIdx = wave[wi].rayIdx;
                ns.alive = (ns.curPwr > pwrTh);
                RayAux na;
                na.hitList = aux[wi].hitList;
                na.total_paths = aux[wi].total_paths;
                if (context.store_paths) {
                    na.rayNodes = aux[wi].rayNodes;
                    PathNode pn; pn.interaction_type = InteractionType::Reflection;
                    pn.face_id = hit.face_id; pn.point = hit.position; pn.direction = newDir;
                    pn.incident_direction = curDir; pn.surface_normal = hit.normal;
                    pn.segment_length_from_previous = Length(Subtract(hit.position, curPt)); pn.valid = true;
                    na.rayNodes.push_back(pn);
                }
                wave[wi].alive = false;
#pragma omp critical(nextwave)
                { nextWave.push_back(ns); nextAux.push_back(na); }
            } else {
                wave[wi].alive = false;
            }
        } // end omp parallel for

        // ── Replace wave with next wave ──
        wave = std::move(nextWave);
        aux = std::move(nextAux);
    }

    std::fprintf(stderr, "\n");

    // ── Merge thread accumulators ──
    for (int t = 0; t < nTh; ++t) {
        auto& tp_ = tp[t];
        auto& th_ = th[t];
        for (int i = 0; i < NRx; ++i) {
            if (th_[i] > 0) {
                result.rx_records[i].total_power_linear += tp_[i];
                result.rx_records[i].ray_hit_count += th_[i];
            }
        }
    }

    for (auto& rec : result.rx_records) {
        if (rec.ray_hit_count > 0) {
            rec.total_power_dBm = 10.0 * std::log10(std::max(1e-30, rec.total_power_linear * txPowerMW));
            result.active_rx_count++;
        } else {
            rec.total_power_dBm = -200.0;
        }
    }
    result.succeeded = true;
    oss.str("");
    oss << "SbrEngine[wavefront]: activeRx=" << result.active_rx_count << "/" << NRx;
    result.trace_lines.push_back(oss.str());
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════
//  v8 GPU Megakernel: single-launch end-to-end SBR
// ═══════════════════════════════════════════════════════════════════════════

SbrCoverageResult SbrEngine::RunMegakernel(const SbrContext& context) const
{
    SbrCoverageResult result;
    const auto& cfg = context.config->sbr;
    const int N = cfg.ray_count;
    const int NRx = (int)context.rx_grid.size();
    const double sphereR = cfg.rx_sphere_radius_m;
    const double txPowerMW = std::pow(10.0, context.tx_power_dBm / 10.0);

    result.total_rays = N;
    result.rx_records.resize(NRx);
    for (int i = 0; i < NRx; ++i) {
        result.rx_records[i].rx_position = context.rx_grid[i];
        result.rx_records[i].rx_index = i;
    }

    // Build Rx grid (same as wavefront)
    RxHashGrid rxGrid;
    rxGrid.Build(context.rx_grid, sphereR);

    // Flatten Rx positions
    std::vector<float> rxFlat(NRx * 3);
    for (int i = 0; i < NRx; ++i) {
        rxFlat[i*3+0] = (float)context.rx_grid[i].x;
        rxFlat[i*3+1] = (float)context.rx_grid[i].y;
        rxFlat[i*3+2] = (float)context.rx_grid[i].z;
    }

    // Build cell offsets + flat data
    int cellCount = rxGrid.nx * rxGrid.ny * rxGrid.nz;
    std::vector<int> cellOffs(cellCount + 1);
    std::vector<int> flatData;
    for (int ci = 0; ci < cellCount; ++ci) {
        cellOffs[ci] = (int)flatData.size();
        for (int rxIdx : rxGrid.flatCells[ci]) flatData.push_back(rxIdx);
    }
    cellOffs[cellCount] = (int)flatData.size();

    std::fprintf(stderr, "[SbrMegakernel] N=%d NRx=%d cells=%d\n", N, NRx, cellCount);

    try {
        SbrGpuPipeline pipeline(*context.scene, *context.config);

        std::vector<float> outPwr(NRx, 0.0f);
        std::vector<int> outHits(NRx, 0);

        pipeline.Run(outPwr, outHits, N,
            rxFlat, cellOffs, flatData,
            (float)rxGrid.cellSize, (float)rxGrid.sphereR,
            (float)rxGrid.ox, (float)rxGrid.oy, (float)rxGrid.oz,
            rxGrid.nx, rxGrid.ny, rxGrid.nz,
            (float)context.tx_point.x, (float)context.tx_point.y, (float)context.tx_point.z);

        for (int i = 0; i < NRx; ++i) {
            if (outHits[i] > 0) {
                result.rx_records[i].total_power_linear = outPwr[i];
                result.rx_records[i].ray_hit_count = outHits[i];
                result.rx_records[i].total_power_dBm =
                    10.0 * std::log10(std::max(1e-30, outPwr[i] * txPowerMW));
                result.active_rx_count++;
            } else {
                result.rx_records[i].total_power_dBm = -200.0;
            }
        }
        result.succeeded = true;
        std::fprintf(stderr, "\n[SbrMegakernel] activeRx=%d/%d\n", result.active_rx_count, NRx);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "\n[SbrMegakernel] FAILED: %s\n", e.what());
        result.trace_lines.push_back(std::string("GPU megakernel failed: ") + e.what());
    }

    return result;
}

// ═══════════════════════════════════════════════════════════════════
// v10: RunPrecise — 确定性几何寻径 (面元: R+T分裂, 棱边: Keller锥)
// 与 Run (蒙卡SBR) 共享 Rx 检测/功率衰减/Fresnel 基础设施
// ═══════════════════════════════════════════════════════════════════

SbrCoverageResult SbrEngine::RunPrecise(const SbrContext& context) const {
    SbrCoverageResult result;
    if (!context.config || !context.scene || !context.scene_query) return result;

    const auto& cfg = context.config->sbr;
    const auto& scfg = context.config->path_search;
    int maxRefl = scfg.max_reflection_count;
    int maxTrans = scfg.max_transmission_count;
    int maxDiff = scfg.max_diffraction_count;
    int maxDepth = scfg.max_path_depth;
    int N_rays = cfg.ray_count;
    double freqHz = context.config->em_solver.frequency_hz;
    double rxSphereM = std::max(0.3, cfg.rx_sphere_radius_m); // 30cm minimum for path capture
    double r2 = rxSphereM * rxSphereM;
    double pwrThresh = std::pow(10.0, -70.0 / 10.0); // -70dB precise mode threshold

    bool hasMat = context.material_db && !context.material_db->empty();
    int RxCount = static_cast<int>(context.rx_grid.size());
    if (RxCount == 0) return result;

    // Rx 位置平面 (快速预检)
    std::vector<Point3> rxPos(RxCount);
    for (int i = 0; i < RxCount; ++i) rxPos[i] = context.rx_grid[i];

    // Per-Rx 路径列表 (线程安全: 每线程独立vector, 最后合并)
    int nThreads = 1;
#ifdef _OPENMP
    nThreads = omp_get_max_threads();
#endif
    std::vector<std::vector<GeometricPath>> threadPaths(nThreads);
    std::vector<std::vector<std::unordered_set<uint64_t>>> threadSigSeen(nThreads);
    for (int t = 0; t < nThreads; ++t) threadSigSeen[t].resize(RxCount);

    // Per-Rx 命中计数
    std::vector<std::atomic<int>> rxHitCounts(RxCount);

    std::fprintf(stderr, "[SbrPrecise] rays=%d R<=%d T<=%d D<=%d depth<=%d rx=%d\n",
                 N_rays, maxRefl, maxTrans, maxDiff, maxDepth, RxCount);

    // ── 射线队列 (wavefront: 按深度分批并行处理) ──
    struct RayState {
        Point3 pos; Vec3 dir;
        double power;
        int depth, nRefl, nTrans, nDiff;
        Point3 prev_pos;  // 上一跳位置 (用于段长度计算)
        std::vector<int> face_ids;
        std::vector<int> wedge_ids;
        std::vector<InteractionType> itypes;
        std::vector<Point3> hit_pts;
        std::vector<Vec3> inc_dirs;     // 入射方向 per bounce
        std::vector<Vec3> out_dirs;     // 出射方向 per bounce
        std::vector<Vec3> normals;      // 面元法线 per bounce
        std::vector<double> seg_lens;   // 段长度 per bounce
    };

    // Fibonacci 球面初始射线
    std::vector<RayState> wave;
    wave.reserve(N_rays);
    double initPwr = 1.0 / N_rays;
    double phi_golden = 3.14159265358979323846 * (3.0 - std::sqrt(5.0));

    for (int i = 0; i < N_rays; ++i) {
        double y = 1.0 - (i / (N_rays - 1.0)) * 2.0;
        double radius = std::sqrt(1.0 - y * y);
        double theta = phi_golden * i;
        Vec3 dir = MakeVec3(radius * std::cos(theta), y, radius * std::sin(theta));
        wave.push_back({context.tx_point, dir, initPwr, 0, 0, 0, 0,
                        context.tx_point, {}, {}, {}, {}, {}, {}, {}, {}});
    }

    const auto& scene = *context.scene;
    const auto& query = *context.scene_query;
    const auto& wedges = scene.wedges;
    int NW = static_cast<int>(wedges.size());

    std::fprintf(stderr, "[SbrPrecise] wave[0]=%zu\n", wave.size());

    int totalBranches = 0;

    // ── LOS 直接检测: Tx→Rx ──
    for (int ri = 0; ri < RxCount; ++ri) {
        VisibilityQueryContext vc;
        if (query.IsVisible(context.tx_point, rxPos[ri], vc)) {
            rxHitCounts[ri]++;
            GeometricPath gp; gp.is_los=true; gp.contains_transmission=false;
            PathNode txNode; txNode.interaction_type=InteractionType::Tx;
            txNode.point=context.tx_point; txNode.valid=true; gp.nodes.push_back(txNode);
            PathNode rxNode; rxNode.interaction_type=InteractionType::Rx;
            rxNode.point=rxPos[ri]; rxNode.valid=true;
            rxNode.segment_length_from_previous=Length(Subtract(rxPos[ri],context.tx_point));
            gp.nodes.push_back(rxNode);
            gp.total_length=rxNode.segment_length_from_previous;
            gp.valid=true; gp.path_signature=BuildPathSignature(gp,*context.config);
            for (int t = 0; t < nThreads; ++t)
                if (threadSigSeen[t][ri].insert(gp.path_signature).second)
                    threadPaths[t].push_back(gp);
        }
    }

    // ── Wavefront 主循环: 逐深度层并行处理 ──
    for (int depth = 0; depth < maxDepth && !wave.empty(); ++depth) {
        std::fprintf(stderr, "[SbrPrecise] depth=%d wave=%zu\r", depth, wave.size());

        // 每线程的下一波 + 路径收集
        std::vector<std::vector<RayState>> threadNext(nThreads);
        std::vector<std::vector<GeometricPath>> threadPathsLocal(nThreads);
        std::vector<std::vector<std::unordered_set<uint64_t>>> threadSigLocal(nThreads);
        for (int t = 0; t < nThreads; ++t) {
            threadSigLocal[t].resize(RxCount);
            threadNext[t].reserve(wave.size() / nThreads * 2);
        }

#pragma omp parallel for schedule(dynamic, 1000)
        for (int wi = 0; wi < static_cast<int>(wave.size()); ++wi) {
            int tid = 0;
#ifdef _OPENMP
            tid = omp_get_thread_num();
#endif
            const RayState& rs = wave[wi];
            if (rs.power < pwrThresh) continue;

            // (Rx检测已移至面元命中后 — 见下文)
            // ── 场景求交 ──
            Ray ray; ray.origin = rs.pos; ray.direction = rs.dir;
            FaceQueryContext fqc;
            fqc.ignored_face_id = rs.face_ids.empty() ? -1 : rs.face_ids.back();
            fqc.ignore_origin_self_hit = true;
            fqc.origin_ignore_distance = 0.01;  // 1cm — 防止浮点自交
            FaceHit hit = query.QueryClosestFaceHit(ray, fqc);
            if (!hit.hit) continue;
            const Face& face = scene.faces[hit.face_id];
            if (face.degenerate) continue;

            double cosI = std::fabs(Dot(rs.dir, hit.normal));
            if (cosI < 1e-4) cosI = 1e-4;
            double refPwr = 0.3;
            if (hasMat) {
                int mh = static_cast<int>(face.surface_eps_r * 100) + static_cast<int>(face.surface_sigma * 1000);
                refPwr = FresnelPowerReflectionCached(cosI, face.surface_eps_r, face.surface_sigma, freqHz, mh);
            }

            // ── Rx 检测 (圆柱管法: 沿射线全程检查) ──
            for (int ri = 0; ri < RxCount; ++ri) {
                Vec3 rxToOrigin = MakeVec3(rxPos[ri].x - rs.pos.x, rxPos[ri].y - rs.pos.y, rxPos[ri].z - rs.pos.z);
                double proj = Dot(rxToOrigin, rs.dir);
                // Rx必须在射线前方
                if (proj <= 0.0 || proj > hit.distance + 0.5) continue;
                // 垂直距离 (圆柱管半径)
                double dPerp2 = Dot(rxToOrigin, rxToOrigin) - proj * proj;
                double rxR2 = 4.0; // 2m接收管半径 (圆柱管法)
                if (dPerp2 > rxR2) continue;
                // 最近点 (沿射线投影)
                Point3 closestPt = MakeVec3(rs.pos.x + proj*rs.dir.x, rs.pos.y + proj*rs.dir.y, rs.pos.z + proj*rs.dir.z);
                // 可见性: 只检查射线起点到该点
                VisibilityQueryContext vc;
                if (!query.IsVisible(rs.pos, closestPt, vc)) continue;

                rxHitCounts[ri]++;
                double finalSeg = Length(Subtract(rxPos[ri], rs.prev_pos));
                double curSeg = Length(Subtract(hit.position, rs.prev_pos));
                GeometricPath gp;
                gp.is_los = (rs.depth == 0);
                gp.contains_transmission = (rs.nTrans > 0);
                PathNode txNode; txNode.interaction_type=InteractionType::Tx;
                txNode.point=context.tx_point; txNode.valid=true; gp.nodes.push_back(txNode);
                for (size_t k=0; k<rs.face_ids.size(); ++k) {
                    PathNode pn; pn.interaction_type=rs.itypes[k];
                    pn.face_id=rs.face_ids[k]; pn.wedge_id=rs.wedge_ids[k];
                    pn.point=rs.hit_pts[k]; pn.valid=true;
                    pn.incident_direction=(k<rs.inc_dirs.size())?rs.inc_dirs[k]:Vec3{};
                    pn.direction=(k<rs.out_dirs.size())?rs.out_dirs[k]:Vec3{};
                    pn.surface_normal=(k<rs.normals.size())?rs.normals[k]:Vec3{};
                    pn.segment_length_from_previous=(k<rs.seg_lens.size())?rs.seg_lens[k]:0.0;
                    if (rs.itypes[k] == InteractionType::Transmission) {
                        int fid = rs.face_ids[k];
                        if (fid >= 0 && fid < (int)scene.faces.size()) {
                            const Face& tf = scene.faces[fid];
                            pn.transmission_semantic_complete = tf.dual_side_material_resolved;
                            bool fromFront = Dot(pn.incident_direction, tf.normal) < 0;
                            pn.medium_in_id=fromFront?tf.front_medium_id:tf.back_medium_id;
                            pn.medium_out_id=fromFront?tf.back_medium_id:tf.front_medium_id;
                            pn.front_medium_id=tf.front_medium_id; pn.back_medium_id=tf.back_medium_id;
                            pn.entered_from_front_side=fromFront;
                        } else { pn.transmission_semantic_complete=true; pn.medium_in_id=0; pn.medium_out_id=1; }
                    }
                    gp.nodes.push_back(pn);
                }
                if (rs.depth > 0) {
                    PathNode last; last.interaction_type=InteractionType::Reflection;
                    last.face_id=hit.face_id; last.point=hit.position; last.valid=true;
                    last.incident_direction=rs.dir;
                    last.direction=Normalize(Subtract(rxPos[ri],hit.position));
                    last.surface_normal=face.normal;
                    last.segment_length_from_previous=curSeg;
                    gp.nodes.push_back(last);
                }
                PathNode rxNode; rxNode.interaction_type=InteractionType::Rx;
                rxNode.point=rxPos[ri]; rxNode.valid=true;
                rxNode.segment_length_from_previous=finalSeg;
                gp.nodes.push_back(rxNode);
                gp.total_length=0;
                for(size_t j=1;j<gp.nodes.size();++j)
                    gp.total_length+=Length(Subtract(gp.nodes[j].point,gp.nodes[j-1].point));
                gp.valid=true;
                gp.path_signature=BuildPathSignature(gp,*context.config);
                if(threadSigLocal[tid][ri].insert(gp.path_signature).second)
                    threadPathsLocal[tid].push_back(gp);
            }

            // ── 绕射检测 (独立棱边遍历, 不依赖面元命中点) ──
            int edgeId = -1; Vec3 eD; Point3 eCenter;
            bool isDiffraction = false;
            if (rs.nDiff < maxDiff && maxDiff > 0) {
                // 对射线求所有棱边的相交: 射线→边垂距 < 阈值 且 交点在射线前方 且 交点在边上
                double eps = 0.15;
                double bestDist = eps;
                const auto& edges = context.scene->edges;
                for (int ei = 0; ei < static_cast<int>(edges.size()); ++ei) {
                    const Edge& e = edges[ei];
                    if (!e.supports_wedge) continue;
                    // 射线-线段最短距离 (两直线间垂距)
                    Vec3 w0 = MakeVec3(rs.pos.x - e.start.x, rs.pos.y - e.start.y, rs.pos.z - e.start.z);
                    Vec3 ed = MakeVec3(e.end.x - e.start.x, e.end.y - e.start.y, e.end.z - e.start.z);
                    double a = Dot(rs.dir, rs.dir), b = Dot(rs.dir, ed), e2 = Dot(ed, ed);
                    double c = Dot(rs.dir, w0), d = -Dot(ed, w0);
                    double denom = a * e2 - b * b;
                    if (std::fabs(denom) < 1e-12) continue;
                    double t = (c * e2 - b * d) / denom;  // 射线参数
                    double s = (a * d - b * c) / denom;   // 边参数
                    if (t <= 0.0 || s < 0.0 || s > 1.0) continue;
                    // 最近点距离
                    Point3 rp = MakeVec3(rs.pos.x + t*rs.dir.x, rs.pos.y + t*rs.dir.y, rs.pos.z + t*rs.dir.z);
                    Point3 ep = MakeVec3(e.start.x + s*ed.x, e.start.y + s*ed.y, e.start.z + s*ed.z);
                    double d2 = (rp.x-ep.x)*(rp.x-ep.x) + (rp.y-ep.y)*(rp.y-ep.y) + (rp.z-ep.z)*(rp.z-ep.z);
                    if (d2 < bestDist * bestDist) { bestDist = std::sqrt(d2); edgeId = ei; }
                }
                if (edgeId >= 0) { const Edge& e=edges[edgeId]; eD=Normalize(e.direction); eCenter=e.midpoint; isDiffraction=true; }
            }

            // (cosI/refPwr 已在命中后计算)

            // ── 反射分支 ──
            if (face.reflection_enabled && rs.nRefl < maxRefl) {
                double segLen = Length(Subtract(hit.position, rs.prev_pos));
                RayState rr = rs; rr.dir = ReflectDir(rs.dir, hit.normal);
                rr.pos = Add(hit.position, Scale(rr.dir, 0.01)); rr.power *= refPwr;
                rr.depth++; rr.nRefl++; rr.prev_pos = hit.position;
                rr.face_ids.push_back(hit.face_id); rr.wedge_ids.push_back(-1);
                rr.itypes.push_back(InteractionType::Reflection); rr.hit_pts.push_back(hit.position);
                rr.inc_dirs.push_back(rs.dir);
                rr.out_dirs.push_back(rr.dir);
                rr.normals.push_back(hit.normal);
                rr.seg_lens.push_back(segLen);
                if (rr.power >= pwrThresh) threadNext[tid].push_back(rr);
            }

            // ── 透射分支 ──
            if (face.transmission_enabled && face.dual_side_material_resolved && rs.nTrans < maxTrans && hasMat) {
                double n2 = std::sqrt(std::max(1.0, face.surface_eps_r));
                double sinT2 = (1.0 - cosI * cosI) / (n2 * n2);
                if (sinT2 < 1.0) {
                    double segLen = Length(Subtract(hit.position, rs.prev_pos));
                    RayState rt = rs; rt.dir = RefractDir(rs.dir, hit.normal, cosI, n2);
                    rt.pos = Add(hit.position, Scale(rt.dir, 0.01)); rt.power *= (1.0 - refPwr);
                    rt.depth++; rt.nTrans++; rt.prev_pos = hit.position;
                    rt.face_ids.push_back(hit.face_id); rt.wedge_ids.push_back(-1);
                    rt.itypes.push_back(InteractionType::Transmission); rt.hit_pts.push_back(hit.position);
                    rt.inc_dirs.push_back(rs.dir);
                    rt.out_dirs.push_back(rt.dir);
                    rt.normals.push_back(hit.normal);
                    rt.seg_lens.push_back(segLen);
                    if (rt.power >= pwrThresh) threadNext[tid].push_back(rt);
                }
            }

            // ── 绕射分支 (Keller锥, 复用老SBR精确公式) ──
            if (isDiffraction && edgeId >= 0) {
                double cB0 = std::fabs(Dot(rs.dir, eD));
                double b0   = std::acos(std::min(1.0, cB0));
                Vec3 pB = Normalize(Cross(rs.dir, eD));
                if (Length(pB) < 1e-9) pB = Normalize(Cross(rs.dir, MakeVec3(0, 1, 0)));
                double segLen = Length(Subtract(eCenter, rs.prev_pos));
                int nSamples = 4; // 4 Keller cone samples
                for (int si = 0; si < nSamples; ++si) {
                    double phi = (si * 0.5 + 0.25) * kPi;
                    Vec3 crossTerm = Cross(eD, pB);
                    Vec3 coneDir = MakeVec3(
                        std::sin(b0) * std::cos(phi) * pB.x + std::cos(b0) * eD.x + std::sin(b0) * std::sin(phi) * crossTerm.x,
                        std::sin(b0) * std::cos(phi) * pB.y + std::cos(b0) * eD.y + std::sin(b0) * std::sin(phi) * crossTerm.y,
                        std::sin(b0) * std::cos(phi) * pB.z + std::cos(b0) * eD.z + std::sin(b0) * std::sin(phi) * crossTerm.z);
                    RayState rd = rs; rd.dir = coneDir;
                    rd.pos = Add(eCenter, Scale(rd.dir, 0.01));
                    rd.power *= 0.1; rd.depth++; rd.nDiff++; rd.prev_pos = eCenter;
                    rd.face_ids.push_back(-1); rd.wedge_ids.push_back(edgeId);
                    rd.itypes.push_back(InteractionType::Diffraction); rd.hit_pts.push_back(eCenter);
                    rd.inc_dirs.push_back(rs.dir); rd.out_dirs.push_back(rd.dir);
                    rd.normals.push_back(eD); rd.seg_lens.push_back(segLen);
                    if (rd.power >= pwrThresh) threadNext[tid].push_back(rd);
                }
            }
        }

        // ── 合并下一波 (上限保护) ──
        wave.clear();
        for (int t = 0; t < nThreads; ++t) {
            totalBranches += static_cast<int>(threadNext[t].size());
            if (wave.size() < 5000000) // cap at 5M rays per wave
                wave.insert(wave.end(), threadNext[t].begin(), threadNext[t].end());
            threadPaths[t].insert(threadPaths[t].end(), threadPathsLocal[t].begin(), threadPathsLocal[t].end());
            threadPathsLocal[t].clear();
            for (int ri = 0; ri < RxCount; ++ri) {
                threadSigSeen[t][ri].insert(threadSigLocal[t][ri].begin(), threadSigLocal[t][ri].end());
                threadSigLocal[t][ri].clear();
            }
        }
#pragma omp parallel for
        for (int t = 0; t < nThreads; ++t) { threadNext[t].clear(); threadNext[t].reserve(wave.size() / nThreads * 2); }
    }

    std::fprintf(stderr, "\n[SbrPrecise] done: branches=%d", totalBranches);

    // ── 合并线程路径 ──
    result.rx_records.resize(RxCount);
    for (int ri = 0; ri < RxCount; ++ri) {
        result.rx_records[ri].rx_index = ri;
        result.rx_records[ri].rx_position = rxPos[ri];
        result.rx_records[ri].ray_hit_count = rxHitCounts[ri].load();
    }
    for (int t = 0; t < nThreads; ++t) {
        for (size_t pi = 0; pi < threadPaths[t].size(); ++pi) {
            const auto& gp = threadPaths[t][pi];
            // 通过前两个交互节点的 face_id 推断 Rx index (简化: 存到所有非空)
            for (int ri = 0; ri < RxCount; ++ri) {
                if (threadSigSeen[t][ri].count(gp.path_signature)) {
                    result.rx_records[ri].paths.push_back(gp);
                    break;
                }
            }
        }
    }

    result.total_rays = N_rays;
    result.active_rx_count = 0;
    int totalPaths = 0;
    for (int ri = 0; ri < RxCount; ++ri) {
        if (result.rx_records[ri].ray_hit_count > 0) result.active_rx_count++;
        totalPaths += static_cast<int>(result.rx_records[ri].paths.size());
    }
    result.succeeded = true;

    std::fprintf(stderr, "[SbrPrecise] done: branches=%d active_rx=%d/%d total_paths=%d\n",
                 totalBranches, result.active_rx_count, RxCount, totalPaths);
    return result;
}

} // namespace rt
