// SbrEngine: forward ray-tracing coverage simulation (v4 C2 + v5 D7 + v5 precision)
// v5 precision: Monte Carlo transmission + Fresnel caching + wedge throttling + power normalization

#include "SbrEngine.h"
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

// ── Rx spatial hash ──
struct RxHashGrid {
    double cellSize=0.6, sphereR=0.3;
    std::unordered_map<uint64_t,std::vector<int>> cells;
    const std::vector<Point3>* rxPositions=nullptr;

    void Build(const std::vector<Point3>& rx, double radius) {
        rxPositions=&rx; sphereR=radius; cellSize=2.0*radius; cells.clear();
        for (int i=0; i<static_cast<int>(rx.size()); ++i) {
            int cx=static_cast<int>(std::floor(rx[i].x/cellSize));
            int cy=static_cast<int>(std::floor(rx[i].y/cellSize));
            int cz=static_cast<int>(std::floor(rx[i].z/cellSize));
            uint64_t key=(uint64_t(cx&0x1FFFFF)<<42)|(uint64_t(cy&0x1FFFFF)<<21)|uint64_t(cz&0x1FFFFF);
            cells[key].push_back(i);
        }
    }
    void CheckSegment(const Point3& a,const Point3& b,std::vector<int>& out) const {
        double r2=sphereR*sphereR;
        double dxb=std::abs(b.x-a.x), dyb=std::abs(b.y-a.y), dzb=std::abs(b.z-a.z);
        // 27邻域覆盖半径≈1.5cell → 跨≤2cell仍可被中点查询覆盖
        bool spansMultiCell = (dxb>2.0*cellSize || dyb>2.0*cellSize || dzb>2.0*cellSize);

        auto queryCell = [&](double mx, double my, double mz) {
            int cx=static_cast<int>(std::floor(mx/cellSize));
            int cy=static_cast<int>(std::floor(my/cellSize));
            int cz=static_cast<int>(std::floor(mz/cellSize));
            for (int dx=-1;dx<=1;++dx) for (int dy=-1;dy<=1;++dy) for (int dz=-1;dz<=1;++dz) {
                uint64_t key=(uint64_t((cx+dx)&0x1FFFFF)<<42)
                           |(uint64_t((cy+dy)&0x1FFFFF)<<21)
                           |uint64_t((cz+dz)&0x1FFFFF);
                auto it=cells.find(key); if (it==cells.end()) continue;
                for (int rxi:it->second) {
                    if (PointToSegmentDistSq((*rxPositions)[rxi],a,b)<=r2) {
                        bool dup=false;
                        for (int prev:out) if (prev==rxi) {dup=true; break;}
                        if (!dup) out.push_back(rxi);
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

// ── 楔边查询 (降频: 仅偶数步执行) ──
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

} // namespace

SbrCoverageResult SbrEngine::Run(const SbrContext& context) const
{
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
    const double originIgnoreDist=context.config->numeric_tolerance.self_hit_ignore_distance; // 循环外预计算

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
    const int reportStep = std::max(1, N / 20); // 每5%报告一次

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(nTh)
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
        int cr=0, ct=0, cd=0, stepIdx=0, noNewHit=0;
        std::vector<int> hitList;

        while (cr+ct+cd<=maxDepth && curPwr>pwrTh)
        {
            stepIdx++;
            Ray ray; ray.origin=curPt; ray.direction=curDir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance=originIgnoreDist;
            FaceHit hit=context.scene_query->QueryClosestFaceHitFast(ray,fqc);

            Point3 segEnd=hit.hit?hit.position:Add(curPt,Scale(curDir,1e6));
            std::vector<int> sHits; rxGrid.CheckSegment(curPt,segEnd,sHits);
            int newHits=0;
            for (int rxi:sHits){bool dup=false;for(int h:hitList){if(h==rxi){dup=true;break;}}if(!dup){hitList.push_back(rxi);myP[rxi]+=curPwr*normFactor;myH[rxi]++;newHits++;}}
            // v7.3 S1: 连续2步无新Rx命中 → 自适应终止
            if (newHits==0) { noNewHit++; if (noNewHit>=2) break; } else noNewHit=0;

            if (!hit.hit) break;
            const Face& face=context.scene->faces[hit.face_id];

            // ── 衍射 (每步全量检测) ──
            if (maxDiff>0 && cd<maxDiff && face.reflection_enabled
                && curPwr>pwrTh10 && !context.scene->wedges.empty()) {
                auto nw=FindNearbyWedges(hit.position,context.scene,wedgeMD,wedgeMS,ri+stepIdx);
                for (int wid:nw) {
                    const Wedge& w=context.scene->wedges[wid];
                    Vec3 eD=Normalize(w.direction); if (Length(eD)<1e-9) continue;
                    double cB0=std::fabs(Dot(curDir,eD)),b0=std::acos(std::min(1.0,cB0));
                    Vec3 pB=Normalize(Cross(curDir,eD)); if (Length(pB)<1e-9) continue;
                    double s1=Length(Subtract(w.center_point,hit.position)); if(s1<0.1)s1=0.1;
                    double sB0=std::sqrt(1.0-cB0*cB0); if(sB0<0.05)sB0=0.05;
                    double nW=(360.0-w.wedge_angle_deg)/180.0; if(nW<0.5)nW=0.5;
                    double k=6.28318530717958647693*freqHz/kC0;
                    double bF=8.0/k*(1.0/(nW*nW))*(1.0/(sB0*sB0)); // v7.3 A3: UTD频率相关绕射系数
                    for (int d=0;d<4;++d){double phi=(d*0.5+0.25)*kPi;
                        Vec3 crossTerm = Cross(eD, pB);
                        Vec3 dd=MakeVec3(
                            std::sin(b0)*std::cos(phi)*pB.x+std::cos(b0)*eD.x+std::sin(b0)*std::sin(phi)*crossTerm.x,
                            std::sin(b0)*std::cos(phi)*pB.y+std::cos(b0)*eD.y+std::sin(b0)*std::sin(phi)*crossTerm.y,
                            std::sin(b0)*std::cos(phi)*pB.z+std::cos(b0)*eD.z+std::sin(b0)*std::sin(phi)*crossTerm.z);
                        Ray dr; dr.origin=w.center_point; dr.direction=dd;
                        FaceHit dh=context.scene_query->QueryClosestFaceHitFast(dr,fqc);
                        Point3 dEnd=dh.hit?dh.position:Add(w.center_point,Scale(dd,10.0));
                        std::vector<int> dH; rxGrid.CheckSegment(w.center_point,dEnd,dH);
                        for (int drxi:dH){bool dup=false;for(int h:hitList){if(h==drxi){dup=true;break;}}
                            if(!dup){hitList.push_back(drxi);
                                double s2=Length(Subtract(context.rx_grid[drxi],w.center_point));if(s2<0.1)s2=0.1;
                                myP[drxi]+=curPwr*bF*(1.0/(s1*s2*(s1+s2)))*normFactor;myH[drxi]++;}}
                    }
                }
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
            if (maxTrans>0 && face.transmission_enabled && ct<maxTrans && hasMat
                && face.dual_side_material_resolved) {
                double r=RandDouble(rng);
                if (r>=refPwr) { // 透射 (概率=1-|Γ|²)
                    double n2=std::sqrt(std::max(1.0,transEpsR));
                    curDir=RefractDir(curDir,hit.normal,cosI,n2);
                    curPt=Add(hit.position,Scale(curDir,0.01));
                    curPwr*=(1.0-refPwr); ct++; continue; // 物理透射衰减
                }
                // 反射 (概率=|Γ|²) — 走下面反射分支 (curPwr*=refPwr)
            }

            // ── 反射 ──
            if (face.reflection_enabled && cr<maxRefl) {
                curDir=ReflectDir(curDir,hit.normal);
                curPt=Add(hit.position,Scale(curDir,0.01));
                curPwr*=refPwr; cr++; continue;
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

} // namespace rt
