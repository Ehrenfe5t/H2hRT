// SbrEngine: SBR geometry pathfinding engine.
// v11: P2P main chain is RunPointToPoint().
//      Modularized components: SbrPathDeduplicator, SbrPathValidator, SbrDiffractionTracer.

#include "SbrEngine.h"
#include "SbrPathDeduplicator.h"      // v11: strict dedup & post-process
#include "SbrPathValidator.h"          // v11: EM-ready validation & residual eval
#include "SbrDiffractionTracer.h"      // v11: analytical Fermat diffraction
#include "SbrPathRefiner.h"            // v11.6: candidate topology -> exact physical path
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

// Fast xorshift RNG (thread-safe, no global state).
inline uint32_t XorShift32(uint32_t& state) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
inline double RandDouble(uint32_t& state) {
    return XorShift32(state) / 4294967296.0; // [0, 1)
}

bool IsFineChannelProfile(const std::string& profile) {
    return profile == "FineChannel" || profile == "fine_channel" || profile == "fine";
}

bool IsDebugValidationProfile(const std::string& profile) {
    return profile == "DebugValidation" || profile == "debug_validation" || profile == "debug";
}

// Fibonacci sphere ray directions.
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

// Rx spatial grid used by dense receiver searches. Current v11 baseline uses
// P2P Rx lists, but this structure is kept as a future coverage extension point.
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
    void CheckSegmentWithRadius(const Point3& a,const Point3& b,double radius,std::vector<int>& out) const {
        if (!rxPositions || rxPositions->empty()) return;
        const double effectiveRadius = std::max(radius, 1.0e-9);
        double r2=effectiveRadius*effectiveRadius;
        double dxb=std::abs(b.x-a.x), dyb=std::abs(b.y-a.y), dzb=std::abs(b.z-a.z);
        bool spansMultiCell = (dxb>2.0*cellSize || dyb>2.0*cellSize || dzb>2.0*cellSize);
        static thread_local std::vector<char> seenMask;
        size_t NRx = rxPositions->size();
        if (seenMask.size() < NRx) seenMask.resize(NRx, 0);
        static thread_local uint8_t genCounter = 0;
        genCounter = (genCounter + 1) & 0xFF;
        if (genCounter == 0) { std::fill(seenMask.begin(), seenMask.end(), 0); genCounter = 1; }

        const int cellRad = std::max(1, static_cast<int>(std::ceil(effectiveRadius / std::max(cellSize, 1.0e-9))));
        auto queryCell = [&](double mx, double my, double mz) {
            int cx=static_cast<int>((mx-ox)/cellSize);
            int cy=static_cast<int>((my-oy)/cellSize);
            int cz=static_cast<int>((mz-oz)/cellSize);
            for (int dx=-cellRad;dx<=cellRad;++dx) for (int dy=-cellRad;dy<=cellRad;++dy) for (int dz=-cellRad;dz<=cellRad;++dz) {
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
            queryCell((a.x+b.x)*0.5, (a.y+b.y)*0.5, (a.z+b.z)*0.5);
        } else {
            double segLen=std::sqrt(dxb*dxb+dyb*dyb+dzb*dzb);
            double step=std::max(effectiveRadius, cellSize*0.5);
            int nSamples=std::max(2, static_cast<int>(std::ceil(segLen/step)));
            for (int s=0;s<=nSamples;++s) {
                double t=static_cast<double>(s)/nSamples;
                queryCell(a.x+(b.x-a.x)*t, a.y+(b.y-a.y)*t, a.z+(b.z-a.z)*t);
            }
        }
    }
    void CheckSegment(const Point3& a,const Point3& b,std::vector<int>& out) const {
        double r2=sphereR*sphereR;
        double dxb=std::abs(b.x-a.x), dyb=std::abs(b.y-a.y), dzb=std::abs(b.z-a.z);
        bool spansMultiCell = (dxb>2.0*cellSize || dyb>2.0*cellSize || dzb>2.0*cellSize);
        // thread_local seen mask avoids repeated linear duplicate checks.
        static thread_local std::vector<char> seenMask;
        size_t NRx = rxPositions ? rxPositions->size() : 0;
        if (seenMask.size() < NRx) seenMask.resize(NRx, 0);
        // Generation counter avoids clearing the mask on every segment check.
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
            // Short segment: one midpoint query is enough for the common case.
            double mx=(a.x+b.x)*0.5, my=(a.y+b.y)*0.5, mz=(a.z+b.z)*0.5;
            queryCell(mx, my, mz);
        } else {
            // Long segment: sample along the segment to avoid skipping cells.
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

// Vector math helpers on the ray-splitting hot path.
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

// Fresnel power reflection cache. thread_local storage avoids OpenMP contention.
// Cache bins: 32 material hash buckets x 20 cos(theta) buckets per thread.
static constexpr int FCACHE_BINS = 20;

double FresnelPowerReflectionCached(double cosI, double epsR, double sigma, double freqHz,
                                     int matHash) {
    thread_local double tlCache[32][FCACHE_BINS] = {}; // per-thread cache
    int bin = std::min(FCACHE_BINS-1, static_cast<int>(cosI * FCACHE_BINS));
    int mh = matHash & 31;
    double& cached = tlCache[mh][bin];
    if (cached > 0.0) return cached;
    // Unpolarized Fresnel power average for the geometric pruning proxy.
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

// Point-to-segment distance.
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

struct SegmentWedgeCoupling {
    int wedge_id = -1;
    Point3 point_on_ray;
    Point3 point_on_wedge;
    double distance = 0.0;
    double ray_t = 0.0;
    double wedge_t = 0.0;
};

double Clamp01(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}

double SegmentSegmentDistance(const Point3& p1, const Point3& q1,
                              const Point3& p2, const Point3& q2,
                              double& sOut, double& tOut,
                              Point3& c1Out, Point3& c2Out) {
    const Vec3 d1 = Subtract(q1, p1);
    const Vec3 d2 = Subtract(q2, p2);
    const Vec3 r = Subtract(p1, p2);
    const double a = Dot(d1, d1);
    const double e = Dot(d2, d2);
    const double f = Dot(d2, r);
    const double eps = 1.0e-12;

    double s = 0.0;
    double t = 0.0;
    if (a <= eps && e <= eps) {
        s = 0.0;
        t = 0.0;
    } else if (a <= eps) {
        s = 0.0;
        t = Clamp01(f / e);
    } else {
        const double c = Dot(d1, r);
        if (e <= eps) {
            t = 0.0;
            s = Clamp01(-c / a);
        } else {
            const double b = Dot(d1, d2);
            const double denom = a * e - b * b;
            if (denom != 0.0) {
                s = Clamp01((b * f - c * e) / denom);
            } else {
                s = 0.0;
            }
            t = (b * s + f) / e;
            if (t < 0.0) {
                t = 0.0;
                s = Clamp01(-c / a);
            } else if (t > 1.0) {
                t = 1.0;
                s = Clamp01((b - c) / a);
            }
        }
    }

    c1Out = Add(p1, Scale(d1, s));
    c2Out = Add(p2, Scale(d2, t));
    sOut = s;
    tOut = t;
    return Length(Subtract(c1Out, c2Out));
}

int FindWedgeIdByEdgeId(const Scene* scene, int edgeId) {
    if (!scene || edgeId < 0) return -1;
    for (const Wedge& wedge : scene->wedges) {
        if (wedge.source_edge_id == edgeId) return wedge.wedge_id;
    }
    return -1;
}

double EstimatePathPowerProxy(const GeometricPath& path) {
    double length = std::max(path.total_length, 1.0e-3);
    double score = -2.0 * std::log(length);
    for (const PathNode& node : path.nodes) {
        switch (node.interaction_type) {
        case InteractionType::Reflection:
            score += std::log(0.45);
            break;
        case InteractionType::Transmission:
            score += std::log(0.35);
            break;
        case InteractionType::Diffraction:
            score += std::log(0.08);
            break;
        default:
            break;
        }
    }
    return score;
}

uint64_t HashPathSimilarityKey(const GeometricPath& path, double lengthTolM) {
    const double tol = std::max(lengthTolM, 1.0e-6);
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](uint64_t v) {
        h ^= v;
        h *= 1099511628211ull;
    };
    const long long lengthBin = static_cast<long long>(std::llround(path.total_length / tol));
    mix(static_cast<uint64_t>(lengthBin));
    for (const PathNode& node : path.nodes) {
        mix(static_cast<uint64_t>(static_cast<int>(node.interaction_type) + 257));
        if (node.face_id >= 0) mix(static_cast<uint64_t>(node.face_id + 4099));
        if (node.wedge_id >= 0) mix(static_cast<uint64_t>(node.wedge_id + 8191));
        if (node.object_id >= 0) mix(static_cast<uint64_t>(node.object_id + 16381));
    }
    return h;
}

Point3 ReflectPointAcrossPlane(const Point3& p, const Point3& planePoint, const Vec3& normal) {
    const Vec3 n = Normalize(normal);
    const double signedDistance = Dot(Subtract(p, planePoint), n);
    return Subtract(p, Scale(n, 2.0 * signedDistance));
}

bool LinePlaneIntersection(const Point3& a, const Point3& b,
                           const Point3& planePoint, const Vec3& normal,
                           Point3& out) {
    const Vec3 ab = Subtract(b, a);
    const Vec3 n = Normalize(normal);
    const double denom = Dot(ab, n);
    if (std::fabs(denom) <= 1.0e-10) return false;
    const double t = Dot(Subtract(planePoint, a), n) / denom;
    out = Add(a, Scale(ab, t));
    return std::isfinite(t);
}

// v11: CountInteractionNodes moved to SbrPathValidator.cpp

std::vector<SegmentWedgeCoupling> FindTubeCoupledWedges(
    const Point3& segStart,
    const Point3& segEnd,
    const Scene* scene,
    double radius,
    int maxCandidates,
    int ignoredWedgeId,
    long long* consideredOut) {
    std::vector<SegmentWedgeCoupling> coupled;
    if (consideredOut) *consideredOut = 0;
    if (!scene || radius <= 0.0 || maxCandidates == 0) return coupled;

    const auto& wa = scene->acceleration.wedge_acceleration;
    const auto& records = wa.wedge_query_records;
    if (records.empty()) return coupled;

    std::vector<int> recordIndices;
    if (wa.grid.nx > 0 && wa.grid.ny > 0 && wa.grid.nz > 0 && wa.grid.cell_size > 0.0 &&
        !wa.grid.cells.empty()) {
        std::vector<char> seen(records.size(), 0);
        const double cellSize = wa.grid.cell_size;
        const double segLen = Length(Subtract(segEnd, segStart));
        const int nSamples = std::max(2, static_cast<int>(std::ceil(segLen / std::max(cellSize * 0.5, radius))));
        const int searchRad = std::max(1, static_cast<int>(std::ceil(radius / cellSize)) + 1);
        const Vec3 segVec = Subtract(segEnd, segStart);
        for (int si = 0; si <= nSamples; ++si) {
            const double u = static_cast<double>(si) / nSamples;
            const Point3 p = Add(segStart, Scale(segVec, u));
            const int cx = static_cast<int>(std::floor((p.x - wa.grid.bounds.min.x) / cellSize));
            const int cy = static_cast<int>(std::floor((p.y - wa.grid.bounds.min.y) / cellSize));
            const int cz = static_cast<int>(std::floor((p.z - wa.grid.bounds.min.z) / cellSize));
            for (int dx = -searchRad; dx <= searchRad; ++dx) {
                const int gx = cx + dx;
                if (gx < 0 || gx >= wa.grid.nx) continue;
                for (int dy = -searchRad; dy <= searchRad; ++dy) {
                    const int gy = cy + dy;
                    if (gy < 0 || gy >= wa.grid.ny) continue;
                    for (int dz = -searchRad; dz <= searchRad; ++dz) {
                        const int gz = cz + dz;
                        if (gz < 0 || gz >= wa.grid.nz) continue;
                        const size_t cellIdx = static_cast<size_t>(gx) * wa.grid.ny * wa.grid.nz
                            + static_cast<size_t>(gy) * wa.grid.nz + static_cast<size_t>(gz);
                        if (cellIdx >= wa.grid.cells.size()) continue;
                        for (int ri : wa.grid.cells[cellIdx]) {
                            if (ri < 0 || ri >= static_cast<int>(records.size())) continue;
                            if (seen[ri]) continue;
                            seen[ri] = 1;
                            recordIndices.push_back(ri);
                        }
                    }
                }
            }
        }
    } else {
        recordIndices.reserve(records.size());
        for (int ri = 0; ri < static_cast<int>(records.size()); ++ri) recordIndices.push_back(ri);
    }

    const double radiusTol = radius + 1.0e-6;
    for (int ri : recordIndices) {
        const auto& record = records[ri];
        if (record.wedge_id < 0 || record.wedge_id == ignoredWedgeId) continue;
        if (record.wedge_id >= static_cast<int>(scene->wedges.size())) continue;
        if (consideredOut) ++(*consideredOut);
        double rayT = 0.0;
        double wedgeT = 0.0;
        Point3 cRay;
        Point3 cWedge;
        const double distance = SegmentSegmentDistance(
            segStart, segEnd, record.segment_start, record.segment_end,
            rayT, wedgeT, cRay, cWedge);
        if (distance > radiusTol) continue;
        SegmentWedgeCoupling hit;
        hit.wedge_id = record.wedge_id;
        hit.point_on_ray = cRay;
        hit.point_on_wedge = cWedge;
        hit.distance = distance;
        hit.ray_t = rayT;
        hit.wedge_t = wedgeT;
        coupled.push_back(hit);
    }

    std::sort(coupled.begin(), coupled.end(),
        [](const SegmentWedgeCoupling& lhs, const SegmentWedgeCoupling& rhs) {
            if (lhs.distance != rhs.distance) return lhs.distance < rhs.distance;
            return lhs.ray_t < rhs.ray_t;
        });
    if (maxCandidates > 0 && static_cast<int>(coupled.size()) > maxCandidates) {
        coupled.resize(static_cast<size_t>(maxCandidates));
    }
    return coupled;
}

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

// Detect whether a face hit is close to one of the triangle edges.
// Returns true when the edge is a scene wedge candidate.
// eps is a geometry tolerance used only by this legacy helper.
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

// ========== v11: migrated to SbrPathDeduplicator.cpp; wrappers kept for legacy call-site compatibility ==========
// TODO: after build verification, remove wrappers and update call sites to use SbrBuildPathSignature() etc.
static double SbrPS(const rt::GeometricPath& p){double L=std::max(p.total_length,1e-3),s=-2*std::log(L);
for(auto& n:p.nodes){switch(n.interaction_type){case rt::InteractionType::Reflection:s+=std::log(.45);break;
case rt::InteractionType::Transmission:s+=std::log(.35);break;case rt::InteractionType::Diffraction:s+=std::log(.08);break;default:break;}}return s;}
static int64_t SbrQ(double v,double st){return(int64_t)std::floor(v/st+0.5);}
static uint64_t SbrSig(const rt::GeometricPath& p,double lT,double pT){uint64_t h=1469598103934665603ull;
auto m=[&](uint64_t v){h^=v;h*=1099511628211ull;};m(p.is_los?1:0);m((uint64_t)p.nodes.size());m((uint64_t)SbrQ(p.total_length,lT));
for(auto& n:p.nodes){m((uint64_t)((int)n.interaction_type+257));m((uint64_t)(n.face_id+4099));m((uint64_t)(n.wedge_id+8191));m((uint64_t)(n.object_id+16381));
m((uint64_t)SbrQ(n.point.x,pT));m((uint64_t)SbrQ(n.point.y,pT));m((uint64_t)SbrQ(n.point.z,pT));
if(n.interaction_type==rt::InteractionType::Transmission){m((uint64_t)(n.medium_in_id+32771));m((uint64_t)(n.medium_out_id+65537));}}return h;}
static bool SbrEq(const rt::GeometricPath& a,const rt::GeometricPath& b,double lT,double pT){if(a.nodes.size()!=b.nodes.size()||a.is_los!=b.is_los)return false;
if(std::fabs(a.total_length-b.total_length)>lT)return false;for(size_t i=0;i<a.nodes.size();++i){auto&na=a.nodes[i];auto&nb=b.nodes[i];
if(na.interaction_type!=nb.interaction_type||na.face_id!=nb.face_id||na.wedge_id!=nb.wedge_id||na.object_id!=nb.object_id)return false;
if(na.interaction_type==rt::InteractionType::Transmission&&(na.medium_in_id!=nb.medium_in_id||na.medium_out_id!=nb.medium_out_id))return false;
double dx=na.point.x-nb.point.x,dy=na.point.y-nb.point.y,dz=na.point.z-nb.point.z;if(std::sqrt(dx*dx+dy*dy+dz*dz)>pT)return false;}return true;}
static uint64_t SbrSK(const rt::GeometricPath& p,double lT,double pT){uint64_t h=1469598103934665603ull;
auto m=[&](uint64_t v){h^=v;h*=1099511628211ull;};m((uint64_t)std::llround(p.total_length/std::max(lT,0.01)));
for(auto& n:p.nodes){m((uint64_t)((int)n.interaction_type+257));if(n.face_id>=0)m((uint64_t)(n.face_id+4099));if(n.wedge_id>=0)m((uint64_t)(n.wedge_id+8191));if(n.object_id>=0)m((uint64_t)(n.object_id+16381));
m((uint64_t)SbrQ(n.point.x,pT));m((uint64_t)SbrQ(n.point.y,pT));m((uint64_t)SbrQ(n.point.z,pT));}return h;}
static void SbrPP(std::vector<rt::GeometricPath>& paths,const rt::AppConfig& cfg){if(paths.empty())return;const double kLT=1e-3,kPT=1e-5;
if(cfg.sbr.enable_path_dedup){std::unordered_map<uint64_t,std::vector<size_t>> b;for(size_t i=0;i<paths.size();++i)b[SbrSig(paths[i],kLT,kPT)].push_back(i);
std::vector<rt::GeometricPath> k;for(auto&[s,ix]:b){std::vector<size_t> kept;for(size_t j=0;j<ix.size();++j){size_t match=k.size();for(size_t kk:kept)if(SbrEq(k[kk],paths[ix[j]],kLT,kPT)){match=kk;break;}if(match<k.size()){k[match].sampling_weight+=paths[ix[j]].sampling_weight;k[match].candidate_support_count+=paths[ix[j]].candidate_support_count;}else{kept.push_back(k.size());k.push_back(std::move(paths[ix[j]]));}}}paths=std::move(k);}
for(auto&p:paths)p.path_signature=SbrSig(p,kLT,kPT);double pC=std::max(cfg.sbr.path_similarity_length_tol_m,0.05);
if(cfg.sbr.enable_path_similarity_pruning&&paths.size()>1){std::unordered_map<uint64_t,size_t> best;for(size_t i=0;i<paths.size();++i){uint64_t key=SbrSK(paths[i],cfg.sbr.path_similarity_length_tol_m,pC);
auto it=best.find(key);if(it==best.end()||SbrPS(paths[i])>SbrPS(paths[it->second]))best[key]=i;}std::vector<rt::GeometricPath> pr;for(auto&kv:best)pr.push_back(std::move(paths[kv.second]));paths=std::move(pr);for(auto&p:paths)p.path_signature=SbrSig(p,kLT,kPT);}
int tN=cfg.sbr.path_top_n_per_rx;if(tN>0&&(int)paths.size()>tN){std::sort(paths.begin(),paths.end(),[](auto&a,auto&b){return SbrPS(a)>SbrPS(b);});paths.resize(tN);}for(size_t i=0;i<paths.size();++i)paths[i].path_id=(int)i;}

static bool BuildDeterministicLosPath(const rt::SceneQuery& query,const rt::AppConfig& config,const rt::Point3& tx,const rt::Point3& rx,rt::GeometricPath& out){
if(!config.path_search.enable_los)return false;rt::Vec3 seg=rt::Subtract(rx,tx);double len=rt::Length(seg);if(len<=config.numeric_tolerance.eps_length)return false;
rt::VisibilityQueryContext vc;if(!query.IsVisible(tx,rx,vc))return false;rt::Vec3 dir=rt::Normalize(seg);
rt::PathNode txNode;txNode.interaction_type=rt::InteractionType::Tx;txNode.point=tx;txNode.direction=dir;txNode.valid=true;
rt::PathNode rxNode;rxNode.interaction_type=rt::InteractionType::Rx;rxNode.point=rx;rxNode.incident_direction=dir;rxNode.segment_length_from_previous=len;rxNode.valid=true;
out=rt::GeometricPath{};out.is_los=true;out.nodes.push_back(txNode);out.nodes.push_back(rxNode);out.total_length=len;out.valid=true;out.path_signature=SbrSig(out,1.0e-3,1.0e-5);return true;}

static rt::Point3 SbrFP(const rt::Point3& tx,const rt::Point3& rx,const rt::Point3& A,const rt::Point3& B,double& t){rt::Vec3 e=rt::Subtract(B,A);double eL2=rt::Dot(e,e);if(eL2<1e-12)return A;
double tTx=rt::Dot(rt::Subtract(tx,A),e)/eL2;rt::Point3 E=rt::Add(A,rt::Scale(e,tTx));double h1=rt::Length(rt::Subtract(tx,E));
double tRx=rt::Dot(rt::Subtract(rx,A),e)/eL2;rt::Point3 C=rt::Add(A,rt::Scale(e,tRx));double h2=rt::Length(rt::Subtract(rx,C));
double hS=h1+h2;if(hS<1e-12){t=tRx;return C;}rt::Point3 dP=rt::Add(C,rt::Scale(rt::Subtract(E,C),h2/hS));t=rt::Dot(rt::Subtract(dP,A),e)/eL2;return dP;}
static void SbrDiff(const rt::Scene& sc,const rt::AppConfig& cfg,const rt::Point3& tx,const std::vector<rt::Point3>& rxG,const rt::SceneQuery& q,std::vector<rt::GeometricPath>& out){if(cfg.sbr.max_diffraction_count<=0||sc.wedges.empty())return;
for(auto& w:sc.wedges){if(!w.diffractable)continue;for(auto& rx:rxG){double t;rt::Point3 dP=SbrFP(tx,rx,w.segment_start,w.segment_end,t);if(t<0||t>1)continue;
double s1=rt::Length(rt::Subtract(dP,tx)),s2=rt::Length(rt::Subtract(rx,dP));if(s1<0.01||s2<0.01)continue;rt::VisibilityQueryContext v;v.ignored_face_id=w.positive_face_id;v.ignored_face_id2=w.negative_face_id;
if(!q.IsVisible(tx,dP,v)||!q.IsVisible(dP,rx,v))continue;rt::GeometricPath g;g.is_los=false;rt::PathNode tN;tN.interaction_type=rt::InteractionType::Tx;tN.point=tx;tN.valid=true;g.nodes.push_back(tN);
rt::PathNode dN;dN.interaction_type=rt::InteractionType::Diffraction;dN.point=dP;dN.wedge_id=w.wedge_id;dN.incident_direction=rt::Normalize(rt::Subtract(dP,tx));dN.direction=rt::Normalize(rt::Subtract(rx,dP));dN.segment_length_from_previous=s1;dN.valid=true;dN.diffraction_diag.s1=s1;dN.diffraction_diag.s2=s2;g.nodes.push_back(dN);
rt::PathNode rN;rN.interaction_type=rt::InteractionType::Rx;rN.point=rx;rN.incident_direction=rt::Normalize(rt::Subtract(rx,dP));rN.segment_length_from_previous=s2;rN.valid=true;g.nodes.push_back(rN);g.total_length=s1+s2;g.valid=true;out.push_back(g);}}}

// Only active when sbr.enabled=true (coverage mode). P2P uses RunPointToPoint().
namespace {

struct SbrPointToPointState {
    Point3 current_point;
    Vec3 current_direction;
    double current_power = 1.0;
    int remaining_reflections = 0;
    int remaining_transmissions = 0;
    int depth = 0;
    int last_face_id = -1;
    int current_medium_id = 0;
    std::vector<PathNode> nodes;
    double cumulative_length = 0.0;
};

// v11: SbrPathContainsTransmission, ValidateSbrPathForEmReady moved to SbrPathValidator.cpp
// v11: SbrAnalyticalFermatPoint, SbrPointOnLargeWedgeSide, TracePointToPointDiffraction moved to SbrDiffractionTracer.cpp

} // namespace

SbrCoverageResult SbrEngine::RunPointToPoint(const SbrContext& context) const
{
    SbrCoverageResult result;
    result.trace_profile = "P2P-SBR";

    if (!context.config || !context.scene || !context.scene_query) {
        result.trace_lines.push_back("SBR P2P 上下文不完整。");
        return result;
    }

    const AppConfig& config = *context.config;
    const auto& cfg = config.sbr;
    const Scene& scene = *context.scene;
    const SceneQuery& query = *context.scene_query;
    const int rayCount = std::max(0, cfg.ray_count);
    const int rxCount = static_cast<int>(context.rx_grid.size());
    result.total_rays = rayCount;
    result.rx_records.resize(rxCount);
    result.path_dedup_enabled = cfg.enable_path_dedup;
    result.path_similarity_pruning_enabled = false;
    result.path_top_n_per_rx = 0;

    if (rayCount <= 0 || rxCount <= 0) {
        result.trace_lines.push_back("SBR P2P 射线数或 Rx 列表为空。");
        result.succeeded = false;
        return result;
    }

    for (int i = 0; i < rxCount; ++i) {
        result.rx_records[i].rx_index = i;
        result.rx_records[i].rx_position = context.rx_grid[i];
    }

    const double sphereR = cfg.rx_sphere_radius_m;
    RxHashGrid rxGrid;
    rxGrid.Build(context.rx_grid, sphereR);

    std::vector<Vec3> rayDirections = GenerateFibonacciRays(rayCount);
    std::vector<int> rayOrder(rayCount);
    for (int i = 0; i < rayCount; ++i) rayOrder[i] = i;
    auto morton3D = [&](const Vec3& d) -> uint32_t {
        auto q = [&](float v) { return (uint32_t)((v + 1.0f) * 511.5f) & 0x3FF; };
        uint32_t x = q((float)d.x), y = q((float)d.y), z = q((float)d.z);
        auto spread = [&](uint32_t v) {
            v = (v | (v << 16)) & 0x030000FF;
            v = (v | (v << 8)) & 0x0300F00F;
            v = (v | (v << 4)) & 0x030C30C3;
            v = (v | (v << 2)) & 0x09249249;
            return v;
        };
        return spread(x) | (spread(y) << 1) | (spread(z) << 2);
    };
    std::sort(rayOrder.begin(), rayOrder.end(),
        [&](int a, int b) { return morton3D(rayDirections[a]) < morton3D(rayDirections[b]); });

#ifdef _OPENMP
    const int threadCount = omp_get_max_threads();
#else
    const int threadCount = 1;
#endif

    std::vector<std::vector<double>> threadPower(threadCount, std::vector<double>(rxCount, 0.0));
    std::vector<std::vector<int>> threadHits(threadCount, std::vector<int>(rxCount, 0));
    std::vector<std::vector<std::vector<GeometricPath>>> threadPaths(
        threadCount, std::vector<std::vector<GeometricPath>>(rxCount));
    std::vector<std::vector<std::unordered_map<uint64_t, std::size_t>>> threadSeen(
        threadCount, std::vector<std::unordered_map<uint64_t, std::size_t>>(rxCount));

    const double powerThreshold = std::pow(10.0, cfg.ray_power_threshold_dB / 10.0);
    const int maxDepth = std::min(cfg.max_ray_depth,
        cfg.max_reflection_count + cfg.max_transmission_count + cfg.max_diffraction_count);
    const double originIgnoreDist = config.numeric_tolerance.self_hit_ignore_distance;
    const double normFactor = 1.0 / static_cast<double>(rayCount);
    const MaterialDatabase* matDb = context.material_db;
    const bool hasMaterialDb = (matDb && !matDb->empty());
    const double freqHz = config.em_solver.frequency_hz;
    double rayTubeAngle = cfg.ray_tube_angle_rad;
    if (rayTubeAngle <= 0.0 && cfg.enable_dynamic_rx_radius) {
        rayTubeAngle = std::sqrt(4.0 * kPi / static_cast<double>(std::max(1, rayCount)));
    }
    const double dynamicRadiusMin = std::max(sphereR, cfg.ray_tube_min_radius_m);
    const double dynamicRadiusMax = (cfg.ray_tube_max_radius_m > 0.0)
        ? cfg.ray_tube_max_radius_m
        : 1.0e6;

    double missSegmentLength = 25.0;
    for (const Point3& rx : context.rx_grid) {
        missSegmentLength = std::max(missSegmentLength, Length(Subtract(rx, context.tx_point)) + sphereR + 10.0);
    }

    std::ostringstream oss;
    oss << "SBR 参数生效：射线数 " << rayCount
        << "，Rx 数量 " << rxCount
        << "，接收球半径 " << sphereR
        << " m，线程数 " << threadCount;
    result.trace_lines.push_back(oss.str());

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) num_threads(threadCount)
#endif
    for (int ri = 0; ri < rayCount; ++ri) {
        int tid = 0;
#ifdef _OPENMP
        tid = omp_get_thread_num();
#endif
        const Vec3& rootDir = rayDirections[rayOrder[ri]];

        SbrPointToPointState root;
        root.current_point = context.tx_point;
        root.current_direction = rootDir;
        root.current_power = 1.0;
        root.remaining_reflections = cfg.max_reflection_count;
        root.remaining_transmissions = cfg.max_transmission_count;
        root.depth = 0;
        root.last_face_id = -1;
        root.current_medium_id = 0;
        root.cumulative_length = 0.0;

        PathNode txNode;
        txNode.interaction_type = InteractionType::Tx;
        txNode.point = context.tx_point;
        txNode.direction = rootDir;
        txNode.valid = true;
        root.nodes.push_back(txNode);

        std::vector<SbrPointToPointState> stack;
        stack.push_back(std::move(root));

        while (!stack.empty()) {
            SbrPointToPointState state = std::move(stack.back());
            stack.pop_back();

            if (state.depth > maxDepth || state.current_power <= powerThreshold) continue;

            Ray ray;
            ray.origin = state.current_point;
            ray.direction = state.current_direction;
            FaceQueryContext fqc;
            fqc.ignored_face_id = state.last_face_id;
            fqc.origin_ignore_distance = originIgnoreDist;
            FaceHit hit = query.QueryClosestFaceHitFast(ray, fqc);
            const Point3 segmentEnd = hit.hit
                ? hit.position
                : Add(state.current_point, Scale(state.current_direction, missSegmentLength));

            std::vector<int> rxHits;
            double effectiveRxRadius = sphereR;
            if (cfg.enable_dynamic_rx_radius && rayTubeAngle > 0.0) {
                const double segmentDistance = hit.hit ? hit.distance : missSegmentLength;
                const double pathDistance = state.cumulative_length + segmentDistance;
                effectiveRxRadius = std::max(sphereR, pathDistance * rayTubeAngle * cfg.ray_tube_radius_scale);
                effectiveRxRadius = Clamp(effectiveRxRadius, dynamicRadiusMin, dynamicRadiusMax);
            }
            rxGrid.CheckSegmentWithRadius(state.current_point, segmentEnd, effectiveRxRadius, rxHits);
            for (int rxIndex : rxHits) {
                if (rxIndex < 0 || rxIndex >= rxCount) continue;

                const Point3& rx = context.rx_grid[rxIndex];
                if (hit.hit) {
                    const double rxSide = Dot(Subtract(rx, hit.position), hit.normal);
                    const double viewSide = Dot(Subtract(state.current_point, hit.position), hit.normal);
                    if (rxSide * viewSide < 0.0) continue;
                }

                Vec3 segVec = Subtract(segmentEnd, state.current_point);
                const double segLen2 = Dot(segVec, segVec);
                double tClosest = 0.0;
                if (segLen2 > 0.0) {
                    tClosest = Dot(Subtract(rx, state.current_point), segVec) / segLen2;
                    if (tClosest < 0.0) tClosest = 0.0;
                    if (tClosest > 1.0) tClosest = 1.0;
                }
                const double lenToClosest = tClosest * std::sqrt(std::max(0.0, segLen2));

                threadPower[tid][rxIndex] += state.current_power * normFactor;
                threadHits[tid][rxIndex] += 1;

                if (context.store_paths) {
                    GeometricPath path;
                    path.is_los = (state.depth == 0);
                    path.nodes = state.nodes;
                    path.contains_transmission = SbrPathContainsTransmission(path);
                    PathNode rxNode;
                    rxNode.interaction_type = InteractionType::Rx;
                    rxNode.point = rx;
                    rxNode.incident_direction = state.current_direction;
                    rxNode.segment_length_from_previous = lenToClosest;
                    rxNode.valid = true;
                    path.nodes.push_back(rxNode);
                    path.total_length = state.cumulative_length + lenToClosest;
                    path.valid = true;
                    path.sampling_weight = normFactor;
                    path.candidate_support_count = 1;
                    path.path_signature = SbrSig(path, 1.0e-3, 1.0e-5);
                    auto& seen = threadSeen[tid][rxIndex];
                    auto seenIt = seen.find(path.path_signature);
                    if (seenIt == seen.end()) {
                        seen[path.path_signature] = threadPaths[tid][rxIndex].size();
                        threadPaths[tid][rxIndex].push_back(std::move(path));
                    } else {
                        threadPaths[tid][rxIndex][seenIt->second].sampling_weight += normFactor;
                        ++threadPaths[tid][rxIndex][seenIt->second].candidate_support_count;
                    }
                }
            }

            if (!hit.hit) continue;
            if (state.depth >= maxDepth) continue;
            if (hit.face_id < 0 || hit.face_id >= static_cast<int>(scene.faces.size())) continue;
            const Face& face = scene.faces[hit.face_id];

            if (face.reflection_enabled && state.remaining_reflections > 0) {
                SbrPointToPointState next;
                next.current_point = hit.position;
                next.current_direction = ReflectDir(state.current_direction, hit.normal);
                next.current_power = state.current_power;
                next.remaining_reflections = state.remaining_reflections - 1;
                next.remaining_transmissions = state.remaining_transmissions;
                next.depth = state.depth + 1;
                next.last_face_id = hit.face_id;
                next.current_medium_id = state.current_medium_id;
                next.cumulative_length = state.cumulative_length + hit.distance;
                next.nodes = state.nodes;

                PathNode node;
                node.interaction_type = InteractionType::Reflection;
                node.point = hit.position;
                node.face_id = hit.face_id;
                node.object_id = hit.object_id;
                node.surface_normal = hit.normal;
                node.incident_direction = state.current_direction;
                node.direction = next.current_direction;
                node.segment_length_from_previous = hit.distance;
                node.valid = true;
                next.nodes.push_back(node);
                stack.push_back(std::move(next));
            }

            if (face.transmission_enabled &&
                face.transmission_semantic_complete &&
                state.remaining_transmissions > 0 &&
                hasMaterialDb) {
                const double dotDN = Dot(state.current_direction, hit.normal);
                const bool fromFront = (dotDN < 0.0);
                const int inMedium = state.current_medium_id;
                const int outMedium = fromFront ? face.back_medium_id : face.front_medium_id;
                if (inMedium >= 0 && outMedium >= 0 && inMedium != outMedium) {
                    const auto p1 = matDb->QueryByName(
                        fromFront ? face.front_material_name : face.back_material_name, freqHz);
                    const auto p2 = matDb->QueryByName(
                        fromFront ? face.back_material_name : face.front_material_name, freqHz);
                    const double n1 = std::sqrt(std::max(1.0, p1.epsilon_r));
                    const double n2 = std::sqrt(std::max(1.0, p2.epsilon_r));
                    const SnellResult sr = SnellRefractV2(state.current_direction, hit.normal, n1, n2);
                    if (sr.valid && !sr.total_internal_reflection) {
                        double r = (n1 - n2) / (n1 + n2);
                        r *= r;

                        SbrPointToPointState next;
                        next.current_point = hit.position;
                        next.current_direction = sr.direction;
                        next.current_power = state.current_power * (1.0 - r);
                        next.remaining_reflections = state.remaining_reflections;
                        next.remaining_transmissions = state.remaining_transmissions - 1;
                        next.depth = state.depth + 1;
                        next.last_face_id = hit.face_id;
                        next.current_medium_id = outMedium;
                        next.cumulative_length = state.cumulative_length + hit.distance;
                        next.nodes = state.nodes;

                        PathNode node;
                        node.interaction_type = InteractionType::Transmission;
                        node.point = hit.position;
                        node.face_id = hit.face_id;
                        node.object_id = hit.object_id;
                        node.surface_normal = hit.normal;
                        node.incident_direction = state.current_direction;
                        node.direction = sr.direction;
                        node.segment_length_from_previous = hit.distance;
                        node.medium_in_id = inMedium;
                        node.medium_out_id = outMedium;
                        node.front_medium_id = face.front_medium_id;
                        node.back_medium_id = face.back_medium_id;
                        node.front_material_id = face.front_material_id;
                        node.back_material_id = face.back_material_id;
                        node.entered_from_front_side = fromFront;
                        node.transmission_semantic_complete = true;
                        node.snell_residual = sr.residual;
                        node.snell_theta_i_rad = sr.theta_i_rad;
                        node.snell_theta_t_rad = sr.theta_t_rad;
                        node.snell_tir = false;
                        node.valid = true;
                        next.nodes.push_back(node);
                        stack.push_back(std::move(next));
                    }
                }
            }
        }
    }

    long long rawPathCount = 0;
    long long finalPathCount = 0;
    long long rejectedForEm = 0;
    long long topologyGroupCount = 0;
    long long geometricallyRefinedCount = 0;
    AppConfig p2pPostConfig = config;
    p2pPostConfig.sbr.enable_path_similarity_pruning = false;
    p2pPostConfig.sbr.path_top_n_per_rx = 0;

    for (int rxIndex = 0; rxIndex < rxCount; ++rxIndex) {
        RxCoverageRecord& rec = result.rx_records[rxIndex];
        for (int tid = 0; tid < threadCount; ++tid) {
            rec.total_power_linear += threadPower[tid][rxIndex];
            rec.ray_hit_count += threadHits[tid][rxIndex];
            for (GeometricPath& path : threadPaths[tid][rxIndex]) {
                rec.paths.push_back(std::move(path));
            }
        }

        rec.paths.erase(
            std::remove_if(rec.paths.begin(), rec.paths.end(),
                [](const GeometricPath& path) { return path.is_los; }),
            rec.paths.end());
        GeometricPath losPath;
        if (BuildDeterministicLosPath(query, config, context.tx_point, rec.rx_position, losPath)) {
            rec.paths.push_back(std::move(losPath));
        }

        TracePointToPointDiffraction(scene, config, context.tx_point, rec.rx_position, rxIndex, query, rec.paths);
        rawPathCount += static_cast<long long>(rec.paths.size());

        const SbrPathRefineStats refineStats = RefineSbrCandidatesForEm(
            rec.paths, scene, query, context.material_db, config.em_solver.frequency_hz);
        topologyGroupCount += refineStats.topology_groups;
        geometricallyRefinedCount += refineStats.refined_paths;
        rejectedForEm += refineStats.rejected_paths;

        SbrPP(rec.paths, p2pPostConfig);

        std::vector<GeometricPath> emReady;
        emReady.reserve(rec.paths.size());
        for (GeometricPath& path : rec.paths) {
            path.contains_transmission = SbrPathContainsTransmission(path);
            EvaluatePathGeometryResidual(path);
            std::string reason;
            if (ValidateSbrPathForEmReady(path, scene, &reason)) {
                emReady.push_back(std::move(path));
            } else {
                ++rejectedForEm;
            }
        }

        std::unordered_map<uint64_t, std::vector<std::size_t>> finalBuckets;
        std::vector<GeometricPath> finalUnique;
        finalUnique.reserve(emReady.size());
        for (GeometricPath& path : emReady) {
            if (path.path_signature == 0) {
                path.path_signature = SbrSig(path, 1.0e-3, 1.0e-5);
            }
            bool duplicate = false;
            auto& bucket = finalBuckets[path.path_signature];
            for (std::size_t keptIndex : bucket) {
                if (SbrEq(finalUnique[keptIndex], path, 1.0e-3, 1.0e-5)) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                bucket.push_back(finalUnique.size());
                finalUnique.push_back(std::move(path));
            } else {
                for (std::size_t keptIndex : bucket) {
                    if (SbrEq(finalUnique[keptIndex], path, 1.0e-3, 1.0e-5)) {
                        finalUnique[keptIndex].sampling_weight += path.sampling_weight;
                        finalUnique[keptIndex].candidate_support_count += path.candidate_support_count;
                        break;
                    }
                }
            }
        }

        rec.paths = std::move(finalUnique);
        for (std::size_t i = 0; i < rec.paths.size(); ++i) {
            rec.paths[i].path_id = static_cast<int>(i);
        }

        rec.total_power_linear = 0.0;
        rec.ray_hit_count = static_cast<int>(rec.paths.size());
        for (const GeometricPath& path : rec.paths) {
            rec.total_power_linear += std::exp(SbrPS(path)) * path.sampling_weight;
        }
        rec.total_power_dBm = context.tx_power_dBm +
            10.0 * std::log10(std::max(1.0e-30, rec.total_power_linear));

        finalPathCount += static_cast<long long>(rec.paths.size());
        if (!rec.paths.empty()) {
            ++result.active_rx_count;
        }
    }

    result.succeeded = true;
    result.rx_paths_recorded = rawPathCount;
    result.physical_topology_group_count = topologyGroupCount;
    result.geometrically_refined_path_count = geometricallyRefinedCount;
    result.geometry_refinement_reject_count = rejectedForEm;
    result.paths_after_postprocess = finalPathCount;
    result.paths_pruned_by_post_dedup = std::max(0LL, rawPathCount - finalPathCount - rejectedForEm);
    result.paths_pruned_by_residual = rejectedForEm;
    oss.str("");
    oss << "SBR 后处理完成：有效 Rx " << result.active_rx_count << "/" << rxCount
        << "，原始候选 " << rawPathCount << "，物理拓扑组 " << topologyGroupCount
        << "，精修成功 " << geometricallyRefinedCount << "，严格去重后 " << finalPathCount
        << "，相似压缩=关闭，TopN=关闭，EM-ready 拒绝 " << rejectedForEm;
    result.trace_lines.push_back(oss.str());
    return result;
}

// The code below is not part of the v11 P2P baseline. It is kept only for
// source compatibility with legacy coverage entry points.
} // namespace rt
