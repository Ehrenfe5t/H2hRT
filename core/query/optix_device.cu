// v8: OptiX device programs — unified raygen + closesthit + anyhit + miss
// RTX 3060 (CC 8.6) + OptiX 7.7

#include <optix.h>

struct GPUFaceRecord {
    unsigned int face_id;
    unsigned int object_id;
    float normal_x, normal_y, normal_z;
    unsigned int propagation_flags;
};

struct UnifiedLaunchParams {
    OptixTraversableHandle traversable;
    const float* ray_origins;
    const float* ray_dirs;
    float tmin;
    float tmax;
    int* hit_flags;
    int* hit_face_ids;
    int* hit_object_ids;
    float* hit_distances;
    float* hit_positions;
    float* hit_normals;
    int* occluded;
    int ray_count;
    int query_type;          // 0=closest-hit, 1=occlusion
    int ignored_face_id;     // face to ignore
    int ignored_face_id2;    // second face to ignore
    const GPUFaceRecord* face_records;
};

extern "C" {
__constant__ UnifiedLaunchParams launch_params;
}

// ── Unified raygen ──

extern "C" __global__ void __raygen__rg()
{
    const unsigned int idx = optixGetLaunchIndex().x;
    if (idx >= static_cast<unsigned int>(launch_params.ray_count))
        return;

    const float3 origin = make_float3(
        launch_params.ray_origins[idx * 3 + 0],
        launch_params.ray_origins[idx * 3 + 1],
        launch_params.ray_origins[idx * 3 + 2]);
    const float3 direction = make_float3(
        launch_params.ray_dirs[idx * 3 + 0],
        launch_params.ray_dirs[idx * 3 + 1],
        launch_params.ray_dirs[idx * 3 + 2]);

    if (launch_params.query_type == 0) {
        // ── Closest-hit query ──
        unsigned int p0 = 0xFFFFFFFFu;
        unsigned int p1 = 0xFFFFFFFFu;
        unsigned int p2 = __float_as_uint(1e30f);
        unsigned int p3 = 0u, p4 = 0u, p5 = 0u;

        optixTrace(
            OPTIX_PAYLOAD_TYPE_DEFAULT,
            launch_params.traversable,
            origin, direction,
            launch_params.tmin, launch_params.tmax,
            0.0f, OptixVisibilityMask(0xFF),
            OPTIX_RAY_FLAG_NONE,
            0u, 0u, 0u,   // SBT hitgroup[0] = closest-hit
            p0, p1, p2, p3, p4, p5
        );

        int face_id = static_cast<int>(p0);
        if (face_id != -1) {
            float t_hit = __uint_as_float(p2);
            launch_params.hit_flags[idx] = 1;
            launch_params.hit_face_ids[idx] = face_id;
            launch_params.hit_object_ids[idx] = static_cast<int>(p1);
            launch_params.hit_distances[idx] = t_hit;
            launch_params.hit_positions[idx * 3 + 0] = origin.x + t_hit * direction.x;
            launch_params.hit_positions[idx * 3 + 1] = origin.y + t_hit * direction.y;
            launch_params.hit_positions[idx * 3 + 2] = origin.z + t_hit * direction.z;
            launch_params.hit_normals[idx * 3 + 0] = __uint_as_float(p3);
            launch_params.hit_normals[idx * 3 + 1] = __uint_as_float(p4);
            launch_params.hit_normals[idx * 3 + 2] = __uint_as_float(p5);
        } else {
            launch_params.hit_flags[idx] = 0;
            launch_params.hit_face_ids[idx] = -1;
            launch_params.hit_object_ids[idx] = -1;
            launch_params.hit_distances[idx] = 0.0f;
            launch_params.hit_positions[idx * 3 + 0] = 0.0f;
            launch_params.hit_positions[idx * 3 + 1] = 0.0f;
            launch_params.hit_positions[idx * 3 + 2] = 0.0f;
            launch_params.hit_normals[idx * 3 + 0] = 0.0f;
            launch_params.hit_normals[idx * 3 + 1] = 0.0f;
            launch_params.hit_normals[idx * 3 + 2] = 0.0f;
        }
    } else {
        // ── Occlusion query ──
        unsigned int p0 = 0u;

        optixTrace(
            OPTIX_PAYLOAD_TYPE_DEFAULT,
            launch_params.traversable,
            origin, direction,
            launch_params.tmin, launch_params.tmax,
            0.0f, OptixVisibilityMask(0xFF),
            OPTIX_RAY_FLAG_TERMINATE_ON_FIRST_HIT,
            1u, 0u, 0u,   // SBT hitgroup[1] = any-hit
            p0
        );

        launch_params.occluded[idx] = (p0 != 0u) ? 1 : 0;
    }
}

// ── Closest-hit program ──

extern "C" __global__ void __closesthit__ch()
{
    const unsigned int prim_idx = optixGetPrimitiveIndex();
    const float hit_t = optixGetRayTmax();

    float current_t = __uint_as_float(optixGetPayload_2());
    if (hit_t < current_t) {
        const GPUFaceRecord& record = launch_params.face_records[prim_idx];
        optixSetPayload_0(record.face_id);
        optixSetPayload_1(record.object_id);
        optixSetPayload_2(__float_as_uint(hit_t));
        optixSetPayload_3(__float_as_uint(record.normal_x));
        optixSetPayload_4(__float_as_uint(record.normal_y));
        optixSetPayload_5(__float_as_uint(record.normal_z));
    }
}

// ── Any-hit: ignore faces (for closest-hit queries) ──

extern "C" __global__ void __anyhit__ignore_faces()
{
    if (launch_params.face_records) {
        const unsigned int prim_idx = optixGetPrimitiveIndex();
        int face_id = static_cast<int>(launch_params.face_records[prim_idx].face_id);
        if (face_id == launch_params.ignored_face_id ||
            face_id == launch_params.ignored_face_id2) {
            optixIgnoreIntersection();
            return;
        }
    }
    // Not an ignored face: accept the intersection (closest-hit will process it)
}

// ── Any-hit: occlusion ──

extern "C" __global__ void __anyhit__ah_occlusion()
{
    if (launch_params.face_records) {
        const unsigned int prim_idx = optixGetPrimitiveIndex();
        int face_id = static_cast<int>(launch_params.face_records[prim_idx].face_id);
        if (face_id == launch_params.ignored_face_id ||
            face_id == launch_params.ignored_face_id2) {
            optixIgnoreIntersection();
            return;
        }
    }
    optixSetPayload_0(1u);
    optixTerminateRay();
}

// ── Miss program ──

extern "C" __global__ void __miss__ms()
{
}
