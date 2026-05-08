// SbrEngine: forward ray-tracing coverage simulation (v4 C2 + v5 D7 + v5 precision)
// v5 precision: Monte Carlo transmission + Fresnel caching + wedge throttling + power normalization

#include "SbrEngine.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <algorithm>
#include <cmath>
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
        double mx=(a.x+b.x)*0.5,my=(a.y+b.y)*0.5,mz=(a.z+b.z)*0.5;
        int cx=static_cast<int>(std::floor(mx/cellSize));
        int cy=static_cast<int>(std::floor(my/cellSize));
        int cz=static_cast<int>(std::floor(mz/cellSize));
        double r2=sphereR*sphereR;
        for (int dx=-1;dx<=1;++dx) for (int dy=-1;dy<=1;++dy) for (int dz=-1;dz<=1;++dz) {
            uint64_t key=(uint64_t((cx+dx)&0x1FFFFF)<<42)
                       |(uint64_t((cy+dy)&0x1FFFFF)<<21)
                       |uint64_t((cz+dz)&0x1FFFFF);
            auto it=cells.find(key); if (it==cells.end()) continue;
            for (int rxi:it->second) {
                if (PointToSegmentDistSq((*rxPositions)[rxi],a,b)<=r2) out.push_back(rxi);
            }
        }
    }
};

// ── Vector math ──
Vec3 ReflectDir(const Vec3& inc, const Vec3& n) {
    double d=Dot(inc,n);
    return MakeVec3(inc.x-2.0*d*n.x, inc.y-2.0*d*n.y, inc.z-2.0*d*n.z);
}
Vec3 RefractDir(const Vec3& inc, const Vec3& n, double cosI, double n2) {
    double sin2t=(1.0-cosI*cosI)/(n2*n2);
    if (sin2t>=1.0) return ReflectDir(inc,n);
    double cosT=std::sqrt(1.0-sin2t), idn=-cosI;
    return Normalize(MakeVec3(
        (inc.x-idn*n.x)/n2 - n.x*cosT,
        (inc.y-idn*n.y)/n2 - n.y*cosT,
        (inc.z-idn*n.z)/n2 - n.z*cosT));
}

// ── Fresnel with cache ──
// 分档: 16 材质 × 20 cosI 档 = 320 条目
static constexpr int FCACHE_BINS = 20;
static double FresnelCache[32][FCACHE_BINS] = {}; // 0=未初始化
static bool FresnelCacheInit = false;

double FresnelPowerReflectionCached(double cosI, double epsR, double sigma, double freqHz,
                                     int matHash) {
    int bin = std::min(FCACHE_BINS-1, static_cast<int>(cosI * FCACHE_BINS));
    int mh = matHash & 31;
    double& cached = FresnelCache[mh][bin];
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
    const int maxTrans=cfg.enable_transmission?cfg.max_transmission_count:0;
    const int maxDiff=cfg.enable_diffraction?cfg.max_diffraction_count:0;
    const double pwrTh=cfg.ray_power_threshold_linear, sphereR=cfg.rx_sphere_radius_m;
    const int NRx=static_cast<int>(context.rx_grid.size());
    const double txPowerMW=context.tx_power_w*1000.0;
    const double normFactor=1.0/static_cast<double>(N);
    const double wedgeMD=cfg.wedge_max_distance_m;
    const int wedgeMS=cfg.wedge_sample_count;

    RxHashGrid rxGrid; rxGrid.Build(context.rx_grid,sphereR);
    result.rx_records.resize(NRx);
    for (int i=0;i<NRx;++i){result.rx_records[i].rx_position=context.rx_grid[i];result.rx_records[i].rx_index=i;}
    result.total_rays=N;
    auto rayDirs=GenerateFibonacciRays(N);

    std::ostringstream oss;
#ifdef _OPENMP
    const int nTh=omp_get_max_threads();
#else
    const int nTh=1;
#endif
    oss<<"SbrEngine: rays="<<N<<" rxCount="<<NRx<<" sphereR="<<sphereR
       <<"m threads="<<nTh<<" trans="<<(cfg.enable_transmission?"on":"off")
       <<" diff="<<(cfg.enable_diffraction?"on":"off");
    result.trace_lines.push_back(oss.str());

    std::vector<std::vector<double>> tp(nTh,std::vector<double>(NRx,0.0));
    std::vector<std::vector<int>> th(nTh,std::vector<int>(NRx,0));

    const auto* matDb=context.material_db;
    const double freqHz=context.config->em_solver.frequency_hz;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) num_threads(nTh)
#endif
    for (int ri=0; ri<N; ++ri)
    {
        int tid=0;
#ifdef _OPENMP
        tid=omp_get_thread_num();
#endif
        auto& myP=tp[tid]; auto& myH=th[tid];
        uint32_t rng=static_cast<uint32_t>(ri*2654435761u+1); // 每射线独立种子

        Point3 curPt=context.tx_point;
        Vec3 curDir=rayDirs[ri];
        double curPwr=1.0;
        int cr=0, ct=0, cd=0, stepIdx=0;
        std::vector<int> hitList;

        while (cr+ct+cd<=maxDepth && curPwr>pwrTh)
        {
            stepIdx++;
            Ray ray; ray.origin=curPt; ray.direction=curDir;
            FaceQueryContext fqc;
            fqc.origin_ignore_distance=context.config->numeric_tolerance.self_hit_ignore_distance;
            FaceHit hit=context.scene_query->QueryClosestFaceHit(ray,fqc);

            Point3 segEnd=hit.hit?hit.position:Add(curPt,Scale(curDir,1e6));
            std::vector<int> sHits; rxGrid.CheckSegment(curPt,segEnd,sHits);
            for (int rxi:sHits){bool dup=false;for(int h:hitList){if(h==rxi){dup=true;break;}}if(!dup){hitList.push_back(rxi);myP[rxi]+=curPwr*normFactor;myH[rxi]++;}}

            if (!hit.hit) break;
            const Face& face=context.scene->faces[hit.face_id];

            // ── 衍射 (降频:仅奇数步, 或stepIdx%2==1) ──
            if (cfg.enable_diffraction && cd<maxDiff && face.reflection_enabled
                && curPwr>pwrTh*10.0 && (stepIdx&1) && !context.scene->wedges.empty()) {
                auto nw=FindNearbyWedges(hit.position,context.scene,wedgeMD,wedgeMS,ri+stepIdx);
                for (int wid:nw) {
                    const Wedge& w=context.scene->wedges[wid];
                    Vec3 eD=Normalize(w.direction); if (Length(eD)<1e-9) continue;
                    double cB0=std::fabs(Dot(curDir,eD)),b0=std::acos(std::min(1.0,cB0));
                    Vec3 pB=Normalize(Cross(curDir,eD)); if (Length(pB)<1e-9) continue;
                    double s1=Length(Subtract(w.center_point,hit.position)); if(s1<0.1)s1=0.1;
                    double sB0=std::sqrt(1.0-cB0*cB0); if(sB0<0.05)sB0=0.05;
                    double nW=(360.0-w.wedge_angle_deg)/180.0; if(nW<0.5)nW=0.5;
                    double bF=0.02*(1.0/(nW*nW))*(1.0/(sB0*sB0));
                    for (int d=0;d<4;++d){double phi=(d*0.5+0.25)*kPi;
                        Vec3 dd=MakeVec3(
                            std::sin(b0)*std::cos(phi)*pB.x+std::cos(b0)*eD.x+std::sin(b0)*std::sin(phi)*Cross(eD,pB).x,
                            std::sin(b0)*std::cos(phi)*pB.y+std::cos(b0)*eD.y,
                            std::sin(b0)*std::cos(phi)*pB.z+std::cos(b0)*eD.z);
                        Ray dr; dr.origin=w.center_point; dr.direction=dd;
                        FaceHit dh=context.scene_query->QueryClosestFaceHit(dr,fqc);
                        Point3 dEnd=dh.hit?dh.position:Add(w.center_point,Scale(dd,10.0));
                        std::vector<int> dH; rxGrid.CheckSegment(w.center_point,dEnd,dH);
                        for (int drxi:dH){bool dup=false;for(int h:hitList){if(h==drxi){dup=true;break;}}
                            if(!dup){hitList.push_back(drxi);
                                double s2=Length(Subtract(context.rx_grid[drxi],w.center_point));if(s2<0.1)s2=0.1;
                                myP[drxi]+=curPwr*bF*(1.0/(s1*s2*(s1+s2)))/0.5*normFactor;myH[drxi]++;}}
                    }
                }
            }

            // ── 材质 ──
            double cosI=std::fabs(Dot(curDir,hit.normal)); if(cosI<0.01)cosI=0.01;
            double refPwr=0.3; double transEpsR=1.0; int matHash=0; bool hasMat=false;
            if (matDb&&!matDb->empty()) {
                std::string mn=face.surface_material_name; if(mn.empty())mn="Concrete";
                MaterialProps mp=matDb->QueryByName(mn,freqHz);
                matHash=static_cast<int>(mp.epsilon_r*100)+static_cast<int>(mp.sigma*1000);
                refPwr=FresnelPowerReflectionCached(cosI,mp.epsilon_r,mp.sigma,freqHz,matHash);
                transEpsR=mp.epsilon_r; hasMat=true;
            }

            // ── 透射 (Monte Carlo: 概率选择+物理功率衰减, 不分裂) ──
            if (cfg.enable_transmission && face.transmission_enabled && ct<maxTrans && hasMat
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

    // 归并
    for (int t=0;t<nTh;++t){auto&tp_=tp[t];auto&th_=th[t];
        for (int i=0;i<NRx;++i){if(th_[i]>0){result.rx_records[i].total_power_linear+=tp_[i];result.rx_records[i].ray_hit_count+=th_[i];}}}

    for (auto& rec:result.rx_records){if(rec.ray_hit_count>0){
        rec.total_power_dBm=10.0*std::log10(std::max(1e-30,rec.total_power_linear*txPowerMW));
        result.active_rx_count++;}}
    result.succeeded=true;
    oss.str("");oss<<"SbrEngine: activeRx="<<result.active_rx_count<<"/"<<NRx;
    result.trace_lines.push_back(oss.str());
    return result;
}

} // namespace rt
