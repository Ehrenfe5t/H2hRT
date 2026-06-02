// 文件目标：
// - 实现模块2统一查询门面的第一版可用逻辑。
//
// 主要功能：
// - 基于批次3的 Face BVH 与 WedgeAcceleration 提供查询能力；
// - 支持最近命中、全部命中、范围命中、遮挡和可见性查询；
// - 显式处理自碰撞抑制与查询上下文约束。

#include "SceneQuery.h"
#include "../common/math/Vec3.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#ifdef __AVX2__
#include <immintrin.h>
#endif

namespace rt {

namespace {

bool IntersectRayAABB(const Ray& ray, const AABB& bounds, double& tMinOut, double& tMaxOut)
{
    if (!bounds.valid)
    {
        return false;
    }

    double tMin = -1.0e300;
    double tMax = 1.0e300;

    const double origins[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
    const double dirs[3] = { ray.direction.x, ray.direction.y, ray.direction.z };
    const double mins[3] = { bounds.min.x, bounds.min.y, bounds.min.z };
    const double maxs[3] = { bounds.max.x, bounds.max.y, bounds.max.z };

    for (int axis = 0; axis < 3; ++axis)
    {
        if (std::fabs(dirs[axis]) <= 1.0e-12)
        {
            if (origins[axis] < mins[axis] || origins[axis] > maxs[axis])
            {
                return false;
            }
            continue;
        }

        double invDir = 1.0 / dirs[axis];
        double t0 = (mins[axis] - origins[axis]) * invDir;
        double t1 = (maxs[axis] - origins[axis]) * invDir;
        if (t0 > t1)
        {
            std::swap(t0, t1);
        }
        if (t0 > tMin)
        {
            tMin = t0;
        }
        if (t1 < tMax)
        {
            tMax = t1;
        }
        if (tMin > tMax)
        {
            return false;
        }
    }

    tMinOut = tMin;
    tMaxOut = tMax;
    return true;
}

bool IntersectRayTriangle(const Ray& ray, const Point3& v0, const Point3& v1, const Point3& v2, double eps, double& tOut)
{
    const Vec3 edge1 = Subtract(v1, v0);
    const Vec3 edge2 = Subtract(v2, v0);
    const Vec3 pvec = MakeVec3(
        ray.direction.y * edge2.z - ray.direction.z * edge2.y,
        ray.direction.z * edge2.x - ray.direction.x * edge2.z,
        ray.direction.x * edge2.y - ray.direction.y * edge2.x);
    const double det = Dot(edge1, pvec);
    if (std::fabs(det) <= eps)
    {
        return false;
    }

    const double invDet = 1.0 / det;
    const Vec3 tvec = Subtract(ray.origin, v0);
    const double u = Dot(tvec, pvec) * invDet;
    if (u < -eps || u > 1.0 + eps)
    {
        return false;
    }

    const Vec3 qvec = MakeVec3(
        tvec.y * edge1.z - tvec.z * edge1.y,
        tvec.z * edge1.x - tvec.x * edge1.z,
        tvec.x * edge1.y - tvec.y * edge1.x);
    const double v = Dot(ray.direction, qvec) * invDet;
    if (v < -eps || u + v > 1.0 + eps)
    {
        return false;
    }

    const double t = Dot(edge2, qvec) * invDet;
    if (t <= eps)
    {
        return false;
    }

    tOut = t;
    return true;
}

bool AcceptFace(const Scene& scene, const Face& face, double distance, const FaceQueryContext& context)
{
    if (face.face_id == context.ignored_face_id)
    {
        return false;
    }
    if (face.face_id == context.ignored_face_id2)
    {
        return false;
    }
    if (face.object_id == context.ignored_object_id)
    {
        return false;
    }
    if (context.ignore_origin_self_hit && distance <= context.origin_ignore_distance)
    {
        return false;
    }
    if (context.only_return_propagation_enabled_faces && face.propagation_flags == FacePropagationNone)
    {
        return false;
    }
    if (context.require_dual_side_material_resolved && !face.dual_side_material_resolved)
    {
        return false;
    }
    static_cast<void>(scene);
    return true;
}

void CollectFaceHitsRecursive(
    const Scene& scene,
    const FaceBVH& bvh,
    int nodeIndex,
    const Ray& ray,
    const FaceQueryContext& context,
    double eps,
    std::vector<FaceHit>& hits)
{
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(bvh.nodes.size()))
    {
        return;
    }

    const FaceBVHNode& node = bvh.nodes[nodeIndex];
    double tMin = 0.0;
    double tMax = 0.0;
    if (!IntersectRayAABB(ray, node.bounds, tMin, tMax))
    {
        return;
    }

    if (node.is_leaf)
    {
        for (int i = 0; i < node.primitive_count; ++i)
        {
            const int primitiveIndex = node.start_index + i;
            if (primitiveIndex < 0 || primitiveIndex >= static_cast<int>(bvh.primitive_face_ids.size()))
            {
                continue;
            }

            const int faceId = bvh.primitive_face_ids[primitiveIndex];
            if (faceId < 0 || faceId >= static_cast<int>(scene.faces.size()))
            {
                continue;
            }

            const Face& face = scene.faces[faceId];
            const Point3& v0 = scene.vertices[face.vertex_index0];
            const Point3& v1 = scene.vertices[face.vertex_index1];
            const Point3& v2 = scene.vertices[face.vertex_index2];

            double distance = 0.0;
            if (!IntersectRayTriangle(ray, v0, v1, v2, eps, distance))
            {
                continue;
            }
            if (!AcceptFace(scene, face, distance, context))
            {
                continue;
            }

            FaceHit hit;
            hit.hit = true;
            hit.face_id = face.face_id;
            hit.object_id = face.object_id;
            hit.distance = distance;
            hit.position = Add(ray.origin, Scale(ray.direction, distance));
            hit.normal = face.normal;
            hits.push_back(hit);
        }
        return;
    }

    CollectFaceHitsRecursive(scene, bvh, node.left_child, ray, context, eps, hits);
    CollectFaceHitsRecursive(scene, bvh, node.right_child, ray, context, eps, hits);
}

#ifdef __AVX2__
// v7.3: AVX2向量化 Möller-Trumbore — 4面一批, 同一条射线广播到4通道
inline void IntersectRayTriangle4_AVX2(
    const Ray& ray, const Point3* v0, const Point3* v1, const Point3* v2,
    double eps, bool* outHit, double* outDist)
{
    __m256d rdx=_mm256_set1_pd(ray.direction.x), rdy=_mm256_set1_pd(ray.direction.y), rdz=_mm256_set1_pd(ray.direction.z);
    __m256d rox=_mm256_set1_pd(ray.origin.x), roy=_mm256_set1_pd(ray.origin.y), roz=_mm256_set1_pd(ray.origin.z);
    __m256d veps=_mm256_set1_pd(eps);

    double v0x[4],v0y[4],v0z[4],v1x[4],v1y[4],v1z[4],v2x[4],v2y[4],v2z[4];
    for(int k=0;k<4;++k){v0x[k]=v0[k].x;v0y[k]=v0[k].y;v0z[k]=v0[k].z;
                          v1x[k]=v1[k].x;v1y[k]=v1[k].y;v1z[k]=v1[k].z;
                          v2x[k]=v2[k].x;v2y[k]=v2[k].y;v2z[k]=v2[k].z;}
    __m256d e1x=_mm256_sub_pd(_mm256_loadu_pd(v1x),_mm256_loadu_pd(v0x));
    __m256d e1y=_mm256_sub_pd(_mm256_loadu_pd(v1y),_mm256_loadu_pd(v0y));
    __m256d e1z=_mm256_sub_pd(_mm256_loadu_pd(v1z),_mm256_loadu_pd(v0z));
    __m256d e2x=_mm256_sub_pd(_mm256_loadu_pd(v2x),_mm256_loadu_pd(v0x));
    __m256d e2y=_mm256_sub_pd(_mm256_loadu_pd(v2y),_mm256_loadu_pd(v0y));
    __m256d e2z=_mm256_sub_pd(_mm256_loadu_pd(v2z),_mm256_loadu_pd(v0z));

    __m256d px=_mm256_fmsub_pd(rdy,e2z,_mm256_mul_pd(rdz,e2y));
    __m256d py=_mm256_fmsub_pd(rdz,e2x,_mm256_mul_pd(rdx,e2z));
    __m256d pz=_mm256_fmsub_pd(rdx,e2y,_mm256_mul_pd(rdy,e2x));
    __m256d det=_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(e1x,px),_mm256_mul_pd(e1y,py)),_mm256_mul_pd(e1z,pz));

    __m256d absDet=_mm256_andnot_pd(_mm256_set1_pd(-0.0),det);
    __m256d maskV=_mm256_cmp_pd(absDet,veps,_CMP_GT_OQ);
    if(_mm256_movemask_pd(maskV)==0){for(int k=0;k<4;++k)outHit[k]=false;return;}

    __m256d invDet=_mm256_div_pd(_mm256_set1_pd(1.0),_mm256_blendv_pd(_mm256_set1_pd(1.0),det,maskV));
    __m256d tvx=_mm256_sub_pd(rox,_mm256_loadu_pd(v0x));
    __m256d tvy=_mm256_sub_pd(roy,_mm256_loadu_pd(v0y));
    __m256d tvz=_mm256_sub_pd(roz,_mm256_loadu_pd(v0z));
    __m256d u=_mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(tvx,px),_mm256_mul_pd(tvy,py)),_mm256_mul_pd(tvz,pz)),invDet);
    maskV=_mm256_and_pd(maskV,_mm256_and_pd(_mm256_cmp_pd(u,_mm256_setzero_pd(),_CMP_GE_OQ),_mm256_cmp_pd(u,_mm256_set1_pd(1.0),_CMP_LE_OQ)));
    if(_mm256_movemask_pd(maskV)==0){for(int k=0;k<4;++k)outHit[k]=false;return;}

    __m256d qx=_mm256_fmsub_pd(tvy,e1z,_mm256_mul_pd(tvz,e1y));
    __m256d qy=_mm256_fmsub_pd(tvz,e1x,_mm256_mul_pd(tvx,e1z));
    __m256d qz=_mm256_fmsub_pd(tvx,e1y,_mm256_mul_pd(tvy,e1x));
    __m256d v=_mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(rdx,qx),_mm256_mul_pd(rdy,qy)),_mm256_mul_pd(rdz,qz)),invDet);
    maskV=_mm256_and_pd(maskV,_mm256_and_pd(_mm256_cmp_pd(v,_mm256_setzero_pd(),_CMP_GE_OQ),_mm256_cmp_pd(_mm256_add_pd(u,v),_mm256_set1_pd(1.0),_CMP_LE_OQ)));
    if(_mm256_movemask_pd(maskV)==0){for(int k=0;k<4;++k)outHit[k]=false;return;}

    __m256d t=_mm256_mul_pd(_mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(e2x,qx),_mm256_mul_pd(e2y,qy)),_mm256_mul_pd(e2z,qz)),invDet);
    maskV=_mm256_and_pd(maskV,_mm256_cmp_pd(t,veps,_CMP_GT_OQ));
    alignas(32) double tArr[4]; _mm256_store_pd(tArr,t);
    int m=_mm256_movemask_pd(maskV);
    for(int k=0;k<4;++k){outHit[k]=(m>>k)&1; if(outHit[k])outDist[k]=tArr[k];}
}
#endif

// v7.3 SBR专用: 近远遍历+提前终止 (不改变仿真结果, 仅加速)
bool CollectClosestFaceHit(
    const Scene& scene, const FaceBVH& bvh, int nodeIndex,
    const Ray& ray, const FaceQueryContext& context, double eps,
    FaceHit& closest, double& closestDist)
{
    if(nodeIndex<0||nodeIndex>=static_cast<int>(bvh.nodes.size())) return false;
    const FaceBVHNode& node=bvh.nodes[nodeIndex];
    double tMin=0,tMax=0;
    if(!IntersectRayAABB(ray,node.bounds,tMin,tMax)) return false;
    if(tMin>=closestDist) return false;
    if(node.is_leaf){
        bool found=false;
        int nPrim=node.primitive_count, si=node.start_index;
        const auto tryFace=[&](int faceId, double dist){
            if(dist>=closestDist) return;
            const Face& face=scene.faces[faceId];
            if(!AcceptFace(scene,face,dist,context)) return;
            closest.hit=true; closest.face_id=face.face_id; closest.object_id=face.object_id;
            closest.distance=dist; closest.position=Add(ray.origin,Scale(ray.direction,dist));
            closest.normal=face.normal; closestDist=dist; found=true;
        };
#ifdef __AVX2__
        int batchEnd=si+(nPrim&~3);
        for(int i=si; i<batchEnd; i+=4){
            Point3 v0[4],v1[4],v2[4]; int fIds[4]; bool ok[4]={}; double d[4];
            for(int k=0;k<4;++k){d[k]=1e100; int fi=i+k;
                if(fi<(int)bvh.primitive_face_ids.size()){fIds[k]=bvh.primitive_face_ids[fi];
                    if(fIds[k]>=0&&fIds[k]<(int)scene.faces.size()){
                        const Face& f=scene.faces[fIds[k]];
                        v0[k]=scene.vertices[f.vertex_index0];v1[k]=scene.vertices[f.vertex_index1];v2[k]=scene.vertices[f.vertex_index2];
                    }else fIds[k]=-1;
                }else fIds[k]=-1;
            }
            IntersectRayTriangle4_AVX2(ray,v0,v1,v2,eps,ok,d);
            for(int k=0;k<4;++k) if(ok[k]&&fIds[k]>=0) tryFace(fIds[k],d[k]);
        }
#else
        int batchEnd=si;
#endif
        for(int i=batchEnd; i<si+nPrim; ++i){
            if(i<0||i>=(int)bvh.primitive_face_ids.size()) continue;
            int faceId=bvh.primitive_face_ids[i];
            if(faceId<0||faceId>=(int)scene.faces.size()) continue;
            const Face& face=scene.faces[faceId];
            double dist=0;
            if(!IntersectRayTriangle(ray,scene.vertices[face.vertex_index0],scene.vertices[face.vertex_index1],scene.vertices[face.vertex_index2],eps,dist)) continue;
            tryFace(faceId,dist);
        }
        return found;
    }
    double tMinL=0,tMaxL=0,tMinR=0,tMaxR=0;
    bool hL=IntersectRayAABB(ray,bvh.nodes[node.left_child].bounds,tMinL,tMaxL);
    bool hR=IntersectRayAABB(ray,bvh.nodes[node.right_child].bounds,tMinR,tMaxR);
    bool found=false;
    if(hL&&hR){
#if defined(__AVX2__) && !defined(_DEBUG)
        _mm_prefetch((const char*)&bvh.nodes[node.right_child], _MM_HINT_T0);
#endif
        if(tMinL<=tMinR){
            found|=CollectClosestFaceHit(scene,bvh,node.left_child,ray,context,eps,closest,closestDist);
            found|=CollectClosestFaceHit(scene,bvh,node.right_child,ray,context,eps,closest,closestDist);
        }else{
            found|=CollectClosestFaceHit(scene,bvh,node.right_child,ray,context,eps,closest,closestDist);
            found|=CollectClosestFaceHit(scene,bvh,node.left_child,ray,context,eps,closest,closestDist);
        }
    }else if(hL){found=CollectClosestFaceHit(scene,bvh,node.left_child,ray,context,eps,closest,closestDist);
    }else if(hR){found=CollectClosestFaceHit(scene,bvh,node.right_child,ray,context,eps,closest,closestDist);}
    return found;
}

} // namespace

/// <summary>
/// 使用场景对象与配置构造查询门面。
/// </summary>
/// <param name="scene">静态场景对象引用。</param>
/// <param name="config">统一应用配置对象引用。</param>
SceneQuery::SceneQuery(const Scene& scene, const AppConfig& config)
    : scene_(scene), config_(config)
{
}

SceneQuery::SceneQuery(const Scene& scene, const AppConfig& config,
                       std::unique_ptr<ISceneAccelerator> accelerator)
    : scene_(scene), config_(config), accelerator_(std::move(accelerator))
{
}

/// <summary>
/// 查询射线正向最近面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>最近有效面元命中结果。</returns>
FaceHit SceneQuery::QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& context) const
{
    // v9 F-1: 路由到快速路径 (BVH早停, 非all-hits→min)
    // GPU single-ray overhead > CPU BVH; batch queries use GPU.
    if (scene_.acceleration.face_acceleration.face_bvh.valid) {
        return QueryClosestFaceHitFast(ray, context);
    }
    // BVH不可用: fallback到all-hits
    std::vector<FaceHit> hits = QueryAllFaceHits(ray, context);
    if (hits.empty()) return FaceHit{};

    FaceHit closest = hits.front();
    for (const FaceHit& hit : hits) {
        if (!hit.hit) continue;
        if (!closest.hit || hit.distance < closest.distance) closest = hit;
    }
    return closest;
}

FaceHit SceneQuery::QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& context) const
{
    // Note: GPU single-ray overhead > CPU BVH. Batch queries use GPU.
    if(!scene_.acceleration.face_acceleration.face_bvh.valid) return FaceHit{};
    FaceHit closest; double closestDist=1e100;
    CollectClosestFaceHit(scene_,scene_.acceleration.face_acceleration.face_bvh,0,
                          ray,context,config_.numeric_tolerance.eps_intersection,closest,closestDist);
    return closest;
}

/// <summary>
/// 查询射线正向全部面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>按距离升序排列的全部有效命中。</returns>
std::vector<FaceHit> SceneQuery::QueryAllFaceHits(const Ray& ray, const FaceQueryContext& context) const
{
    std::vector<FaceHit> hits;
    if (!scene_.acceleration.face_acceleration.face_bvh.valid)
    {
        return hits;
    }

    CollectFaceHitsRecursive(
        scene_,
        scene_.acceleration.face_acceleration.face_bvh,
        0,
        ray,
        context,
        config_.numeric_tolerance.eps_intersection,
        hits);

    std::sort(hits.begin(), hits.end(),
        [](const FaceHit& lhs, const FaceHit& rhs)
        {
            return lhs.distance < rhs.distance;
        });
    return hits;
}

/// <summary>
/// 查询指定距离区间内的全部面元命中。
/// </summary>
/// <param name="ray">输入射线。</param>
/// <param name="minDistance">最小距离。</param>
/// <param name="maxDistance">最大距离。</param>
/// <param name="context">局部查询约束。</param>
/// <returns>区间内有效命中集合。</returns>
std::vector<FaceHit> SceneQuery::QueryFaceHitsInRange(
    const Ray& ray,
    double minDistance,
    double maxDistance,
    const FaceQueryContext& context) const
{
    std::vector<FaceHit> allHits = QueryAllFaceHits(ray, context);
    std::vector<FaceHit> rangeHits;
    for (const FaceHit& hit : allHits)
    {
        if (hit.distance >= minDistance && hit.distance <= maxDistance)
        {
            rangeHits.push_back(hit);
        }
    }
    return rangeHits;
}

/// <summary>
/// 判断两点开区间是否被有效面阻挡。
/// </summary>
/// <param name="start">线段起点。</param>
/// <param name="end">线段终点。</param>
/// <param name="context">可见性查询上下文。</param>
/// <returns>true 表示存在阻挡；false 表示无遮挡。</returns>
bool SceneQuery::IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const
{
    // Note: GPU single-ray overhead > CPU. Batch queries use GPU.
    const Vec3 delta = Subtract(end, start);
    const double length = Length(delta);
    if (length <= config_.numeric_tolerance.eps_length)
    {
        return false;
    }

    const Vec3 direction = Normalize(delta);
    Point3 rayOrigin = start;
    if (context.ignore_origin_attached_face)
    {
        rayOrigin = Add(rayOrigin, Scale(direction, context.origin_offset_distance));
    }
    Point3 rayEnd = end;
    if (context.ignore_target_attached_face)
    {
        rayEnd = Add(rayEnd, Scale(direction, -context.target_shrink_distance));
    }

    const double effectiveLength = Length(Subtract(rayEnd, rayOrigin));
    if (effectiveLength <= config_.numeric_tolerance.eps_length)
    {
        return false;
    }

    Ray ray;
    ray.origin = rayOrigin;
    ray.direction = direction;

    FaceQueryContext faceContext;
    faceContext.ignored_face_id = context.ignored_face_id;
    faceContext.ignored_face_id2 = context.ignored_face_id2;
    faceContext.ignored_object_id = context.ignored_object_id;
    faceContext.ignore_origin_self_hit = true;
    faceContext.origin_ignore_distance = config_.numeric_tolerance.self_hit_ignore_distance;

    const std::vector<FaceHit> hits = QueryFaceHitsInRange(ray, 0.0, effectiveLength, faceContext);
    return !hits.empty();
}

/// <summary>
/// 判断两点是否可见。
/// </summary>
/// <param name="start">起点。</param>
/// <param name="end">终点。</param>
/// <param name="context">可见性查询上下文。</param>
/// <returns>true 表示可见；false 表示不可见。</returns>
bool SceneQuery::IsVisible(const Point3& start, const Point3& end, const VisibilityQueryContext& context) const
{
    return !IsOccluded(start, end, context);
}

/// <summary>
/// 查询楔边候选集合。
/// </summary>
/// <param name="origin">当前参考点。</param>
/// <param name="context">楔边查询上下文。</param>
/// <returns>候选楔边集合。</returns>
std::vector<WedgeCandidate> SceneQuery::QueryCandidateWedges(const Point3& origin, const WedgeQueryContext& context) const
{
    std::vector<WedgeCandidate> candidates;
    const auto& wa = scene_.acceleration.wedge_acceleration;
    const auto& records = wa.wedge_query_records;
    const double maxDist = config_.path_search.wedge_max_distance_m;

    // v9 step26: 使用均匀网格空间索引, 仅查询origin附近的cell
    if (wa.grid.nx > 0 && wa.grid.cell_size > 0.0) {
        // 找到origin所在cell
        int cx = static_cast<int>((origin.x - wa.grid.bounds.min.x) / wa.grid.cell_size);
        int cy = static_cast<int>((origin.y - wa.grid.bounds.min.y) / wa.grid.cell_size);
        int cz = static_cast<int>((origin.z - wa.grid.bounds.min.z) / wa.grid.cell_size);

        // 搜索半径 (cell数) — 至少1, 覆盖maxDist范围
        int searchRad = std::max(1, static_cast<int>(maxDist / wa.grid.cell_size) + 1);

        // 遍历邻域cells
        for (int dz = -searchRad; dz <= searchRad; ++dz) {
            int nz = cz + dz;
            if (nz < 0 || nz >= wa.grid.nz) continue;
            for (int dy = -searchRad; dy <= searchRad; ++dy) {
                int ny = cy + dy;
                if (ny < 0 || ny >= wa.grid.ny) continue;
                for (int dx = -searchRad; dx <= searchRad; ++dx) {
                    int nx = cx + dx;
                    if (nx < 0 || nx >= wa.grid.nx) continue;

                    size_t cellIdx = static_cast<size_t>(nx) * wa.grid.ny * wa.grid.nz
                                   + static_cast<size_t>(ny) * wa.grid.nz + nz;
                    if (cellIdx >= wa.grid.cells.size()) continue;

                    for (int wi : wa.grid.cells[cellIdx]) {
                        if (wi < 0 || wi >= static_cast<int>(records.size())) continue;
                        const auto& record = records[wi];

                        if (record.wedge_id == context.ignored_wedge_id) continue;
                        if (context.avoid_recent_wedge && record.wedge_id == context.recent_wedge_id) continue;
                        if (context.avoid_adjacent_wedge_to_recent_face &&
                            (record.positive_face_id == context.recent_face_id ||
                             record.negative_face_id == context.recent_face_id)) continue;

                        const Vec3 delta = Subtract(record.center_point, origin);
                        if (Length(delta) > maxDist) continue;

                        WedgeCandidate candidate;
                        candidate.wedge_id = record.wedge_id;
                        candidate.source_edge_id = record.source_edge_id;
                        candidate.center_point = record.center_point;
                        candidate.direction = record.direction;
                        candidate.length = record.length;
                        candidate.wedge_angle_deg = record.wedge_angle_deg;
                        candidate.positive_face_id = record.positive_face_id;
                        candidate.negative_face_id = record.negative_face_id;
                        candidates.push_back(candidate);
                    }
                }
            }
        }
    } else {
        // grid不可用 → fallback线性扫描
        for (const WedgeQueryRecord& record : records) {
            if (record.wedge_id == context.ignored_wedge_id) continue;
            if (context.avoid_recent_wedge && record.wedge_id == context.recent_wedge_id) continue;
            if (context.avoid_adjacent_wedge_to_recent_face &&
                (record.positive_face_id == context.recent_face_id ||
                 record.negative_face_id == context.recent_face_id)) continue;

            const Vec3 delta = Subtract(record.center_point, origin);
            if (Length(delta) > maxDist) continue;

            WedgeCandidate candidate;
            candidate.wedge_id = record.wedge_id;
            candidate.source_edge_id = record.source_edge_id;
            candidate.center_point = record.center_point;
            candidate.direction = record.direction;
            candidate.length = record.length;
            candidate.wedge_angle_deg = record.wedge_angle_deg;
            candidate.positive_face_id = record.positive_face_id;
            candidate.negative_face_id = record.negative_face_id;
            candidates.push_back(candidate);
        }
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [&origin](const WedgeCandidate& lhs, const WedgeCandidate& rhs)
        {
            const double lhsDistance = Length(Subtract(lhs.center_point, origin));
            const double rhsDistance = Length(Subtract(rhs.center_point, origin));
            if (lhsDistance != rhsDistance)
            {
                return lhsDistance < rhsDistance;
            }
            return lhs.length > rhs.length;
        });

    const int maxWedgeCands = config_.path_search.wedge_max_candidates;
    if (static_cast<int>(candidates.size()) > maxWedgeCands)
    {
        candidates.resize(static_cast<std::size_t>(maxWedgeCands));
    }

    return candidates;
}

std::vector<FaceHit> SceneQuery::QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const
{
    if (accelerator_) return accelerator_->QueryClosestFaceHitBatch(rays, ctx);
    // CPU fallback: loop
    std::vector<FaceHit> results; results.reserve(rays.size());
    for (const auto& ray : rays) results.push_back(QueryClosestFaceHit(ray, ctx));
    return results;
}

std::vector<bool> SceneQuery::IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                                const VisibilityQueryContext& ctx) const
{
    if (accelerator_) return accelerator_->IsOccludedBatch(starts, ends, ctx);
    std::vector<bool> results; results.reserve(starts.size());
    for (size_t i = 0; i < starts.size(); ++i) results.push_back(IsOccluded(starts[i], ends[i], ctx));
    return results;
}

std::vector<std::vector<int>> SceneQuery::QueryRxHitsBatch(
    const std::vector<double>& seg_starts_flat,
    const std::vector<double>& seg_ends_flat,
    const ISceneAccelerator::RxGridQueryParams& grid) const
{
    if (accelerator_)
        return accelerator_->QueryRxHitsBatch(seg_starts_flat, seg_ends_flat, grid);
    int N = static_cast<int>(seg_starts_flat.size() / 3);
    return std::vector<std::vector<int>>(N);
}


// v9 F-3: backend query diagnostics report
std::string SceneQuery::GetBackendDiagnostics() const {
    std::ostringstream ss;
    ss << "Backend: " << (accelerator_ ? accelerator_->BackendName() : "CPU_FaceBVH");
    if (accelerator_) {
        ss << " | queries: closest=" << accelerator_->closest_hit_queries
           << " all=" << accelerator_->all_hits_queries
           << " occ=" << accelerator_->occlusion_queries
           << " | all-hits=" << (accelerator_->SupportsAllHits() ? "yes" : "NO")
           << " double=" << (accelerator_->UsesDoublePrecision() ? "yes" : "no");
    }
    return ss.str();
}
} // namespace rt
