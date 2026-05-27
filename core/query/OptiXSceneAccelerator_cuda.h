// v8: CUDA-specific declarations for OptiXSceneAccelerator
// This header is included ONLY by OptiXSceneAccelerator.cu (nvcc)
// It contains no project C++ dependencies — safe for nvcc EDG front-end
#pragma once

#include <cuda_runtime.h>
#include <optix_types.h>

// ── GPU data structures ──

struct GPUVertex {
    float x, y, z;
};

struct GPUFaceRecord {
    unsigned int face_id;
    unsigned int object_id;
    float normal_x, normal_y, normal_z;
    unsigned int propagation_flags;
};

// SBT record data
struct SbtRecordData {
    int material_index;
};

struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) HitgroupSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    SbtRecordData data;
};

struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) RaygenSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
};

struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) MissSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
};

// Launch params
struct ClosestHitParams {
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
    int ray_count;
    const GPUFaceRecord* face_records;
};

struct OcclusionParams {
    OptixTraversableHandle traversable;
    const float* ray_origins;
    const float* ray_dirs;
    float tmin;
    float tmax;
    int* occluded;
    int ray_count;
};

// ── CUDA-side method declarations ──
// (All defined in OptiXSceneAccelerator.cu, declared in OptiXSceneAccelerator.h)
// This header provides the types needed by the .cu file without heavy includes.

namespace rt {

class OptiXSceneAccelerator_cuda {
public:
    // These mirror the private methods of OptiXSceneAccelerator
    // The .cu file can access them since they're all private to the same class
};

} // namespace rt
