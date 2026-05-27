// v8 Phase 4: OptiXSceneAccelerator — C++ side (compiled by MSVC, not nvcc)
// Contains methods that access project data structures (Scene, Face, AppConfig)

#include "OptiXSceneAccelerator.h"

// Full includes for project types (safe - compiled by cl.exe, not nvcc)
#include "SceneQuery.h"
#include "../scene/Scene.h"
#include "../common/config/AppConfig.h"

#include <cuda_runtime.h>
#include <cstdio>
#include <cmath>

namespace rt {

// ═══════════════════════════════════════════════════════════
//  BuildFromScene (accesses Scene internals)
// ═══════════════════════════════════════════════════════════

void OptiXSceneAccelerator::BuildFromScene(const Scene& scene)
{
    CleanupGpuBuffers();
    if (scene.faces.empty()) { built_ = false; return; }
    UploadSceneGeometry(scene);
    BuildAccel();
    built_ = true;
    fprintf(stdout, "[OptiXAccel] Built GAS: %d triangles\n", triangle_count_);
}

// ═══════════════════════════════════════════════════════════
//  Constructor / Destructor
// ═══════════════════════════════════════════════════════════

OptiXSceneAccelerator::OptiXSceneAccelerator(const Scene& scene, const AppConfig& config)
    : scene_(scene), config_(config)
{
    device_id_ = config_.acceleration.gpu_device_id;
    max_batch_size_ = static_cast<size_t>(config_.acceleration.gpu_batch_size);

    // Quick CUDA self-test before OptiX init
    {
        int* d_tmp = nullptr;
        cudaError_t e = cudaMalloc(reinterpret_cast<void**>(&d_tmp), sizeof(int));
        if (e == cudaSuccess) {
            cudaMemset(d_tmp, 0xAB, sizeof(int));
            int host_val;
            cudaMemcpy(&host_val, d_tmp, sizeof(int), cudaMemcpyDeviceToHost);
            cudaFree(d_tmp);
            fprintf(stdout, "[OptiXAccel] CUDA self-test OK (val=0x%x)\n", host_val);
        } else {
            fprintf(stderr, "[OptiXAccel] CUDA self-test FAILED: %s\n", cudaGetErrorString(e));
        }
    }

    try {
        InitOptiX();
        BuildFromScene(scene);
    } catch (...) {
        CleanupGpuBuffers();
        CleanupOptiX();
        throw;
    }
}

OptiXSceneAccelerator::~OptiXSceneAccelerator()
{
    CleanupGpuBuffers();
    CleanupOptiX();
}

// ═══════════════════════════════════════════════════════════
//  Scene geometry upload (needs access to Scene internals)
// ═══════════════════════════════════════════════════════════

void OptiXSceneAccelerator::UploadSceneGeometry(const Scene& scene)
{
    const int num_faces = static_cast<int>(scene.faces.size());
    triangle_count_ = num_faces;
    vertex_count_ = num_faces * 3;

    // Build CPU-side buffers
    std::vector<GPUVertex> vertices(vertex_count_);
    std::vector<unsigned int> indices(vertex_count_);
    std::vector<GPUFaceRecord> face_records(num_faces);

    for (int i = 0; i < num_faces; ++i) {
        const Face& face = scene.faces[i];
        const int vi = i * 3;

        const Point3& v0 = scene.vertices[face.vertex_index0];
        const Point3& v1 = scene.vertices[face.vertex_index1];
        const Point3& v2 = scene.vertices[face.vertex_index2];

        vertices[vi + 0] = { static_cast<float>(v0.x), static_cast<float>(v0.y), static_cast<float>(v0.z) };
        vertices[vi + 1] = { static_cast<float>(v1.x), static_cast<float>(v1.y), static_cast<float>(v1.z) };
        vertices[vi + 2] = { static_cast<float>(v2.x), static_cast<float>(v2.y), static_cast<float>(v2.z) };

        indices[vi + 0] = static_cast<unsigned int>(vi + 0);
        indices[vi + 1] = static_cast<unsigned int>(vi + 1);
        indices[vi + 2] = static_cast<unsigned int>(vi + 2);

        face_records[i].face_id = static_cast<unsigned int>(face.face_id);
        face_records[i].object_id = static_cast<unsigned int>(face.object_id);
        face_records[i].normal_x = static_cast<float>(face.normal.x);
        face_records[i].normal_y = static_cast<float>(face.normal.y);
        face_records[i].normal_z = static_cast<float>(face.normal.z);
        face_records[i].propagation_flags = face.propagation_flags;
    }

    UploadGeometryData(
        reinterpret_cast<const float*>(vertices.data()),
        indices.data(),
        face_records.data(),
        num_faces);
}

// ═══════════════════════════════════════════════════════════
//  Query methods
// ═══════════════════════════════════════════════════════════

FaceHit OptiXSceneAccelerator::QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const
{
    if (!built_) return FaceHit{};

    float origins[3] = {
        static_cast<float>(ray.origin.x),
        static_cast<float>(ray.origin.y),
        static_cast<float>(ray.origin.z)
    };
    float dirs[3] = {
        static_cast<float>(ray.direction.x),
        static_cast<float>(ray.direction.y),
        static_cast<float>(ray.direction.z)
    };
    float tMin = static_cast<float>(ctx.origin_ignore_distance > 0 ? ctx.origin_ignore_distance : 0.001);

    std::lock_guard<std::mutex> lock(launch_mutex_);

    int hitFlag = 0, hitFaceId = -1;
    float hitDist = 0.0f, hitPos[3] = {0, 0, 0}, hitNorm[3] = {0, 0, 0};

    LaunchClosestHitQuery(origins, dirs, 1, tMin, 1e30f,
        &hitFlag, &hitFaceId, &hitDist, hitPos, hitNorm,
        ctx.ignored_face_id, ctx.ignored_face_id2);

    FaceHit hit;
    if (hitFlag) {
        hit.hit = true;
        hit.face_id = hitFaceId;
        hit.distance = static_cast<double>(hitDist);
        hit.position.x = static_cast<double>(hitPos[0]);
        hit.position.y = static_cast<double>(hitPos[1]);
        hit.position.z = static_cast<double>(hitPos[2]);
        hit.normal.x = static_cast<double>(hitNorm[0]);
        hit.normal.y = static_cast<double>(hitNorm[1]);
        hit.normal.z = static_cast<double>(hitNorm[2]);
    }
    return hit;
}

FaceHit OptiXSceneAccelerator::QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const
{
    return QueryClosestFaceHit(ray, ctx);
}

std::vector<FaceHit> OptiXSceneAccelerator::QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const
{
    std::vector<FaceHit> hits;
    FaceHit closest = QueryClosestFaceHit(ray, ctx);
    if (closest.hit) hits.push_back(closest);
    return hits;
}

std::vector<FaceHit> OptiXSceneAccelerator::QueryFaceHitsInRange(
    const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const
{
    std::vector<FaceHit> hits;
    FaceHit closest = QueryClosestFaceHit(ray, ctx);
    if (closest.hit && closest.distance >= tMin && closest.distance <= tMax)
        hits.push_back(closest);
    return hits;
}

bool OptiXSceneAccelerator::IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const
{
    if (!built_) return false;

    float origins[3] = {
        static_cast<float>(start.x),
        static_cast<float>(start.y),
        static_cast<float>(start.z)
    };
    float dx = static_cast<float>(end.x - start.x);
    float dy = static_cast<float>(end.y - start.y);
    float dz = static_cast<float>(end.z - start.z);
    float len = std::sqrt(dx * dx + dy * dy + dz * dz);
    float dirs[3] = { 1.0f, 0.0f, 0.0f };
    if (len > 1e-6f) {
        dirs[0] = dx / len; dirs[1] = dy / len; dirs[2] = dz / len;
    }

    std::lock_guard<std::mutex> lock(launch_mutex_);
    int occluded = 0;
    float tMin = static_cast<float>(ctx.origin_offset_distance);
    LaunchOcclusionQuery(origins, dirs, 1, tMin, len, &occluded,
        ctx.ignored_face_id, ctx.ignored_face_id2);
    return occluded != 0;
}

std::vector<FaceHit> OptiXSceneAccelerator::QueryClosestFaceHitBatch(
    const std::vector<Ray>& rays, const FaceQueryContext& ctx) const
{
    if (rays.empty() || !built_) return {};

    const int n = static_cast<int>(rays.size());
    std::lock_guard<std::mutex> lock(launch_mutex_);

    float* origins = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));
    float* dirs = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));
    int* hitFlags = static_cast<int*>(std::malloc(n * sizeof(int)));
    int* hitFaceIds = static_cast<int*>(std::malloc(n * sizeof(int)));
    float* hitDistances = static_cast<float*>(std::malloc(n * sizeof(float)));
    float* hitPositions = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));
    float* hitNormals = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));

    float tmin = static_cast<float>(ctx.origin_ignore_distance > 0 ? ctx.origin_ignore_distance : 0.001);

    for (int i = 0; i < n; ++i) {
        origins[i * 3 + 0] = static_cast<float>(rays[i].origin.x);
        origins[i * 3 + 1] = static_cast<float>(rays[i].origin.y);
        origins[i * 3 + 2] = static_cast<float>(rays[i].origin.z);
        dirs[i * 3 + 0] = static_cast<float>(rays[i].direction.x);
        dirs[i * 3 + 1] = static_cast<float>(rays[i].direction.y);
        dirs[i * 3 + 2] = static_cast<float>(rays[i].direction.z);
    }

    LaunchClosestHitQuery(origins, dirs, n, tmin, 1e30f,
        hitFlags, hitFaceIds, hitDistances, hitPositions, hitNormals,
        ctx.ignored_face_id, ctx.ignored_face_id2);

    std::vector<FaceHit> results; results.reserve(n);
    for (int i = 0; i < n; ++i) {
        FaceHit hit;
        if (hitFlags[i]) {
            hit.hit = true;
            hit.face_id = hitFaceIds[i];
            hit.distance = static_cast<double>(hitDistances[i]);
            hit.position.x = static_cast<double>(hitPositions[i * 3 + 0]);
            hit.position.y = static_cast<double>(hitPositions[i * 3 + 1]);
            hit.position.z = static_cast<double>(hitPositions[i * 3 + 2]);
            hit.normal.x = static_cast<double>(hitNormals[i * 3 + 0]);
            hit.normal.y = static_cast<double>(hitNormals[i * 3 + 1]);
            hit.normal.z = static_cast<double>(hitNormals[i * 3 + 2]);
        }
        results.push_back(hit);
    }

    std::free(origins); std::free(dirs);
    std::free(hitFlags); std::free(hitFaceIds);
    std::free(hitDistances); std::free(hitPositions); std::free(hitNormals);
    return results;
}

std::vector<bool> OptiXSceneAccelerator::IsOccludedBatch(
    const std::vector<Point3>& starts, const std::vector<Point3>& ends,
    const VisibilityQueryContext& ctx) const
{
    if (starts.empty() || !built_) return {};

    const int n = static_cast<int>(starts.size());
    std::lock_guard<std::mutex> lock(launch_mutex_);

    float* origins = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));
    float* dirs = static_cast<float*>(std::malloc(n * 3 * sizeof(float)));
    int* occluded = static_cast<int*>(std::malloc(n * sizeof(int)));

    float tMin = static_cast<float>(ctx.origin_offset_distance);

    for (int i = 0; i < n; ++i) {
        origins[i * 3 + 0] = static_cast<float>(starts[i].x);
        origins[i * 3 + 1] = static_cast<float>(starts[i].y);
        origins[i * 3 + 2] = static_cast<float>(starts[i].z);

        float dx = static_cast<float>(ends[i].x - starts[i].x);
        float dy = static_cast<float>(ends[i].y - starts[i].y);
        float dz = static_cast<float>(ends[i].z - starts[i].z);
        float len = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (len > 1e-6f) {
            dirs[i * 3 + 0] = dx / len;
            dirs[i * 3 + 1] = dy / len;
            dirs[i * 3 + 2] = dz / len;
        } else {
            dirs[i * 3 + 0] = 1.0f; dirs[i * 3 + 1] = 0.0f; dirs[i * 3 + 2] = 0.0f;
        }
    }

    LaunchOcclusionQuery(origins, dirs, n, tMin, 1e30f, occluded,
        ctx.ignored_face_id, ctx.ignored_face_id2);

    std::vector<bool> results(n, false);
    for (int i = 0; i < n; ++i) results[i] = (occluded[i] != 0);

    std::free(origins); std::free(dirs); std::free(occluded);
    return results;
}

} // namespace rt
