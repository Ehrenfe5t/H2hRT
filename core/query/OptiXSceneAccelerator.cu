// v8 Phase 4: OptiXSceneAccelerator CUDA implementation
// Pure CUDA/OptiX code - no project C++ header dependencies
// Constructor/destructor/BuildFromScene/UploadSceneGeometry are in OptiXSceneAccelerator.cpp

#define OPTIX_STUBS_INCLUDE_IMPL

#include "OptiXSceneAccelerator.h"

#include <optix.h>
#include <optix_stubs.h>
#include <optix_function_table.h>
#include <optix_function_table_definition.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <new>
#include <stdexcept>
#include <mutex>

// OptiX stack size utilities
#include <optix_stack_size.h>

namespace rt {

// Error-check macros
#define CUDA_CHECK(call) do { \
    cudaError_t err_ = call; \
    if (err_ != cudaSuccess) { \
        std::fprintf(stderr, "[OptiXAccel] CUDA error %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err_)); \
        throw std::runtime_error(cudaGetErrorString(err_)); \
    } \
} while (0)

#define OPTIX_CHECK(call) do { \
    OptixResult res_ = call; \
    if (res_ != OPTIX_SUCCESS) { \
        std::fprintf(stderr, "[OptiXAccel] OptiX error %s:%d: %d\n", \
                __FILE__, __LINE__, static_cast<int>(res_)); \
        throw std::runtime_error("OptiX call failed"); \
    } \
} while (0)

// ── Upload raw geometry data (called from BuildFromScene in .cpp) ──

void OptiXSceneAccelerator::UploadGeometryData(
    const float* vertices, const unsigned int* indices,
    const GPUFaceRecord* face_records, int num_faces)
{
    triangle_count_ = num_faces;
    vertex_count_ = num_faces * 3;

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_vertices_),
        vertex_count_ * sizeof(GPUVertex)));
    CUDA_CHECK(cudaMemcpy(d_vertices_, vertices,
        vertex_count_ * sizeof(GPUVertex), cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_indices_),
        vertex_count_ * sizeof(unsigned int)));
    CUDA_CHECK(cudaMemcpy(d_indices_, indices,
        vertex_count_ * sizeof(unsigned int), cudaMemcpyHostToDevice));

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_face_records_),
        num_faces * sizeof(GPUFaceRecord)));
    CUDA_CHECK(cudaMemcpy(d_face_records_, face_records,
        num_faces * sizeof(GPUFaceRecord), cudaMemcpyHostToDevice));
}

// ── OptiX initialization ──

void OptiXSceneAccelerator::InitOptiX()
{
    CUDA_CHECK(cudaSetDevice(device_id_));
    CUDA_CHECK(cudaFree(nullptr));
    OPTIX_CHECK(optixInit());
    CreateContext();
    CreateModule();
    CreateProgramGroups();
    CreatePipeline();
    BuildSbt();
}

void OptiXSceneAccelerator::CreateContext()
{
    OptixDeviceContextOptions ctx_opts = {};
    ctx_opts.logCallbackFunction = [](unsigned int level, const char* tag,
                                       const char* message, void*) {
        if (level >= 4) return;
        std::fprintf(stderr, "[OptiX][%s] %s\n", tag, message);
    };
    ctx_opts.logCallbackLevel = 3;
    OPTIX_CHECK(optixDeviceContextCreate(nullptr, &ctx_opts, &optix_context_));

    unsigned int rtCoreVersion = 0;
    optixDeviceContextGetProperty(optix_context_, OPTIX_DEVICE_PROPERTY_RTCORE_VERSION,
                                   &rtCoreVersion, sizeof(rtCoreVersion));
    std::fprintf(stdout, "[OptiXAccel] RT Core version: %u\n", rtCoreVersion);
}

void OptiXSceneAccelerator::CreateModule()
{
    const char* ptx_path = "x64/Release/optix_device.ptx";
    std::string ptx_source;

    FILE* f = std::fopen(ptx_path, "rb");
    if (!f) f = std::fopen("optix_device.ptx", "rb");
    if (!f) f = std::fopen("../x64/Release/optix_device.ptx", "rb");
    if (!f) {
        throw std::runtime_error(
            "[OptiXAccel] Cannot open PTX file 'optix_device.ptx'.");
    }
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    ptx_source.resize(sz + 1, '\0');
    std::fread(&ptx_source[0], 1, sz, f);
    std::fclose(f);

    OptixModuleCompileOptions module_compile_opts = {};
    module_compile_opts.maxRegisterCount = OPTIX_COMPILE_DEFAULT_MAX_REGISTER_COUNT;
    module_compile_opts.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
    module_compile_opts.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;

    OptixPipelineCompileOptions pipeline_compile_opts = {};
    pipeline_compile_opts.usesMotionBlur = false;
    pipeline_compile_opts.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pipeline_compile_opts.numPayloadValues = 6;
    pipeline_compile_opts.numAttributeValues = 2;
    pipeline_compile_opts.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipeline_compile_opts.pipelineLaunchParamsVariableName = "launch_params";

    char log_buffer[4096];
    size_t log_size = sizeof(log_buffer);

    OptixResult res = optixModuleCreate(
        optix_context_, &module_compile_opts, &pipeline_compile_opts,
        ptx_source.c_str(), ptx_source.size(),
        log_buffer, &log_size, &optix_module_);

    if (res != OPTIX_SUCCESS) {
        std::fprintf(stderr, "[OptiXAccel] Module creation failed:\n%s\n", log_buffer);
        throw std::runtime_error("OptiX module creation failed");
    }
}

void OptiXSceneAccelerator::CreateProgramGroups()
{
    OptixProgramGroupOptions pg_opts = {};
    OptixProgramGroupDesc pg_desc = {};
    char log_buffer[4096];
    size_t log_size;

    std::memset(&pg_desc, 0, sizeof(pg_desc));
    pg_desc.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
    pg_desc.raygen.module = optix_module_;
    pg_desc.raygen.entryFunctionName = "__raygen__rg";
    log_size = sizeof(log_buffer);
    OPTIX_CHECK(optixProgramGroupCreate(optix_context_, &pg_desc, 1,
        &pg_opts, log_buffer, &log_size, &raygen_pg_));

    std::memset(&pg_desc, 0, sizeof(pg_desc));
    pg_desc.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
    pg_desc.miss.module = optix_module_;
    pg_desc.miss.entryFunctionName = "__miss__ms";
    log_size = sizeof(log_buffer);
    OPTIX_CHECK(optixProgramGroupCreate(optix_context_, &pg_desc, 1,
        &pg_opts, log_buffer, &log_size, &miss_pg_));

    std::memset(&pg_desc, 0, sizeof(pg_desc));
    pg_desc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
    pg_desc.hitgroup.moduleCH = optix_module_;
    pg_desc.hitgroup.entryFunctionNameCH = "__closesthit__ch";
    pg_desc.hitgroup.moduleAH = optix_module_;
    pg_desc.hitgroup.entryFunctionNameAH = "__anyhit__ignore_faces";
    log_size = sizeof(log_buffer);
    OPTIX_CHECK(optixProgramGroupCreate(optix_context_, &pg_desc, 1,
        &pg_opts, log_buffer, &log_size, &closest_hit_pg_));

    std::memset(&pg_desc, 0, sizeof(pg_desc));
    pg_desc.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
    pg_desc.hitgroup.moduleAH = optix_module_;
    pg_desc.hitgroup.entryFunctionNameAH = "__anyhit__ah_occlusion";
    log_size = sizeof(log_buffer);
    OPTIX_CHECK(optixProgramGroupCreate(optix_context_, &pg_desc, 1,
        &pg_opts, log_buffer, &log_size, &any_hit_pg_));
}

void OptiXSceneAccelerator::CreatePipeline()
{
    OptixProgramGroup pgs[] = { raygen_pg_, miss_pg_, closest_hit_pg_, any_hit_pg_ };
    int num_pgs = static_cast<int>(sizeof(pgs) / sizeof(pgs[0]));

    OptixPipelineCompileOptions pipeline_compile_opts = {};
    pipeline_compile_opts.usesMotionBlur = false;
    pipeline_compile_opts.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pipeline_compile_opts.numPayloadValues = 6;
    pipeline_compile_opts.numAttributeValues = 2;
    pipeline_compile_opts.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipeline_compile_opts.pipelineLaunchParamsVariableName = "launch_params";

    OptixPipelineLinkOptions pipeline_link_opts = {};
    pipeline_link_opts.maxTraceDepth = 1;

    char log_buffer[4096];
    size_t log_size = sizeof(log_buffer);

    OPTIX_CHECK(optixPipelineCreate(
        optix_context_, &pipeline_compile_opts, &pipeline_link_opts,
        pgs, static_cast<unsigned int>(num_pgs),
        log_buffer, &log_size, &optix_pipeline_));

    OptixStackSizes stack_sizes = {};
    for (int i = 0; i < num_pgs; ++i) {
        OPTIX_CHECK(optixUtilAccumulateStackSizes(pgs[i], &stack_sizes, optix_pipeline_));
    }
    unsigned int direct_cc = 0, direct_dc = 0, continuation = 0;
    OPTIX_CHECK(optixUtilComputeStackSizes(&stack_sizes, 1, 0, 0,
        &direct_cc, &direct_dc, &continuation));
    OPTIX_CHECK(optixPipelineSetStackSize(optix_pipeline_,
        direct_cc, direct_dc, continuation, 1));
}

void OptiXSceneAccelerator::BuildSbt()
{
    RaygenSbtRecord raygen_record;
    OPTIX_CHECK(optixSbtRecordPackHeader(raygen_pg_, &raygen_record));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&sbt_.raygenRecord),
        sizeof(RaygenSbtRecord)));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(sbt_.raygenRecord),
        &raygen_record, sizeof(RaygenSbtRecord), cudaMemcpyHostToDevice));

    MissSbtRecord miss_record;
    OPTIX_CHECK(optixSbtRecordPackHeader(miss_pg_, &miss_record));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&sbt_.missRecordBase),
        sizeof(MissSbtRecord)));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(sbt_.missRecordBase),
        &miss_record, sizeof(MissSbtRecord), cudaMemcpyHostToDevice));
    sbt_.missRecordStrideInBytes = sizeof(MissSbtRecord);
    sbt_.missRecordCount = 1;

    HitgroupSbtRecord hit_records[2];
    OPTIX_CHECK(optixSbtRecordPackHeader(closest_hit_pg_, &hit_records[0]));
    hit_records[0].data.material_index = 0;
    OPTIX_CHECK(optixSbtRecordPackHeader(any_hit_pg_, &hit_records[1]));
    hit_records[1].data.material_index = 0;

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&sbt_.hitgroupRecordBase),
        2 * sizeof(HitgroupSbtRecord)));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(sbt_.hitgroupRecordBase),
        hit_records, 2 * sizeof(HitgroupSbtRecord), cudaMemcpyHostToDevice));
    sbt_.hitgroupRecordStrideInBytes = sizeof(HitgroupSbtRecord);
    sbt_.hitgroupRecordCount = 2;
}

// ── BVH (GAS) build ──

void OptiXSceneAccelerator::BuildAccel()
{
    OptixBuildInput build_input = {};
    build_input.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;

    CUdeviceptr d_vertices_ptr = reinterpret_cast<CUdeviceptr>(d_vertices_);
    CUdeviceptr d_indices_ptr = reinterpret_cast<CUdeviceptr>(d_indices_);

    build_input.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
    build_input.triangleArray.vertexStrideInBytes = sizeof(GPUVertex);
    build_input.triangleArray.numVertices = static_cast<unsigned int>(vertex_count_);
    build_input.triangleArray.vertexBuffers = &d_vertices_ptr;

    build_input.triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
    build_input.triangleArray.indexStrideInBytes = 3 * sizeof(unsigned int);
    build_input.triangleArray.numIndexTriplets = static_cast<unsigned int>(triangle_count_);
    build_input.triangleArray.indexBuffer = d_indices_ptr;

    unsigned int input_flags[1] = { OPTIX_GEOMETRY_FLAG_NONE };
    build_input.triangleArray.flags = input_flags;
    build_input.triangleArray.numSbtRecords = 1;

    OptixAccelBuildOptions accel_opts = {};
    accel_opts.buildFlags = OPTIX_BUILD_FLAG_PREFER_FAST_TRACE;
    accel_opts.operation = OPTIX_BUILD_OPERATION_BUILD;

    OptixAccelBufferSizes buffer_sizes;
    OPTIX_CHECK(optixAccelComputeMemoryUsage(optix_context_, &accel_opts,
        &build_input, 1, &buffer_sizes));

    CUdeviceptr d_temp_buffer;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_temp_buffer),
        buffer_sizes.tempSizeInBytes));

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_gas_output_),
        buffer_sizes.outputSizeInBytes));

    OPTIX_CHECK(optixAccelBuild(optix_context_, nullptr, &accel_opts,
        &build_input, 1, d_temp_buffer, buffer_sizes.tempSizeInBytes,
        d_gas_output_, buffer_sizes.outputSizeInBytes,
        &gas_handle_, nullptr, 0));

    CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_temp_buffer)));
}

// ── Core launch ──

void OptiXSceneAccelerator::LaunchClosestHitQuery(
    const float* origins, const float* dirs, int rayCount,
    float tMin, float tMax,
    int* hitFlags, int* hitFaceIds,
    float* hitDistances, float* hitPositions, float* hitNormals,
    int ignoredFaceId, int ignoredFaceId2) const
{
    float *d_origins, *d_dirs, *d_hit_distances, *d_hit_positions, *d_hit_normals;
    int *d_hit_flags, *d_hit_face_ids, *d_hit_object_ids;

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_origins), rayCount * 3 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_dirs), rayCount * 3 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_flags), rayCount * sizeof(int)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_face_ids), rayCount * sizeof(int)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_object_ids), rayCount * sizeof(int)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_distances), rayCount * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_positions), rayCount * 3 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_normals), rayCount * 3 * sizeof(float)));

    CUDA_CHECK(cudaMemcpy(d_origins, origins, rayCount * 3 * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_dirs, dirs, rayCount * 3 * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_hit_flags, 0, rayCount * sizeof(int)));

    UnifiedLaunchParams params;
    params.traversable = gas_handle_;
    params.ray_origins = d_origins;
    params.ray_dirs = d_dirs;
    params.tmin = tMin;
    params.tmax = tMax;
    params.hit_flags = d_hit_flags;
    params.hit_face_ids = d_hit_face_ids;
    params.hit_object_ids = d_hit_object_ids;
    params.hit_distances = d_hit_distances;
    params.hit_positions = d_hit_positions;
    params.hit_normals = d_hit_normals;
    params.ray_count = rayCount;
    params.query_type = 0;  // closest-hit
    params.ignored_face_id = ignoredFaceId;
    params.ignored_face_id2 = ignoredFaceId2;
    params.face_records = d_face_records_;

    CUdeviceptr d_params;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_params), sizeof(UnifiedLaunchParams)));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_params), &params,
        sizeof(UnifiedLaunchParams), cudaMemcpyHostToDevice));

    OPTIX_CHECK(optixLaunch(optix_pipeline_, nullptr, d_params,
        sizeof(UnifiedLaunchParams), &sbt_,
        static_cast<unsigned int>(rayCount), 1, 1));

    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(hitFlags, d_hit_flags, rayCount * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(hitFaceIds, d_hit_face_ids, rayCount * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(hitDistances, d_hit_distances, rayCount * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(hitPositions, d_hit_positions, rayCount * 3 * sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(hitNormals, d_hit_normals, rayCount * 3 * sizeof(float), cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_origins)); CUDA_CHECK(cudaFree(d_dirs));
    CUDA_CHECK(cudaFree(d_hit_flags)); CUDA_CHECK(cudaFree(d_hit_face_ids));
    CUDA_CHECK(cudaFree(d_hit_object_ids)); CUDA_CHECK(cudaFree(d_hit_distances));
    CUDA_CHECK(cudaFree(d_hit_positions)); CUDA_CHECK(cudaFree(d_hit_normals));
    CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_params)));
}

void OptiXSceneAccelerator::LaunchOcclusionQuery(
    const float* origins, const float* dirs, int rayCount,
    float tMin, float tMax, int* occluded,
    int ignoredFaceId, int ignoredFaceId2) const
{
    float *d_origins, *d_dirs;
    int *d_occluded;

    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_origins), rayCount * 3 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_dirs), rayCount * 3 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_occluded), rayCount * sizeof(int)));

    CUDA_CHECK(cudaMemcpy(d_origins, origins, rayCount * 3 * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_dirs, dirs, rayCount * 3 * sizeof(float), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_occluded, 0, rayCount * sizeof(int)));

    UnifiedLaunchParams params;
    params.traversable = gas_handle_;
    params.ray_origins = d_origins;
    params.ray_dirs = d_dirs;
    params.tmin = tMin;
    params.tmax = tMax;
    params.occluded = d_occluded;
    params.ray_count = rayCount;
    params.query_type = 1;  // occlusion
    params.ignored_face_id = ignoredFaceId;
    params.ignored_face_id2 = ignoredFaceId2;
    params.face_records = d_face_records_;  // needed by any-hit for ignore check

    CUdeviceptr d_params;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_params), sizeof(UnifiedLaunchParams)));
    CUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>(d_params), &params,
        sizeof(UnifiedLaunchParams), cudaMemcpyHostToDevice));

    OPTIX_CHECK(optixLaunch(optix_pipeline_, nullptr, d_params,
        sizeof(UnifiedLaunchParams), &sbt_,
        static_cast<unsigned int>(rayCount), 1, 1));

    CUDA_CHECK(cudaDeviceSynchronize());
    CUDA_CHECK(cudaMemcpy(occluded, d_occluded, rayCount * sizeof(int), cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_origins)); CUDA_CHECK(cudaFree(d_dirs));
    CUDA_CHECK(cudaFree(d_occluded));
    CUDA_CHECK(cudaFree(reinterpret_cast<void*>(d_params)));
}

// ── Cleanup ──

void OptiXSceneAccelerator::CleanupGpuBuffers()
{
    if (d_vertices_)       { cudaFree(reinterpret_cast<void*>(d_vertices_));     d_vertices_ = nullptr; }
    if (d_indices_)        { cudaFree(reinterpret_cast<void*>(d_indices_));      d_indices_ = nullptr; }
    if (d_face_records_)   { cudaFree(reinterpret_cast<void*>(d_face_records_)); d_face_records_ = nullptr; }
    if (d_gas_output_)     { cudaFree(reinterpret_cast<void*>(d_gas_output_));   d_gas_output_ = 0; }
    if (d_rx_positions_cached_) { cudaFree(reinterpret_cast<void*>(d_rx_positions_cached_)); d_rx_positions_cached_ = nullptr; }
    if (d_cell_offsets_cached_) { cudaFree(reinterpret_cast<void*>(d_cell_offsets_cached_)); d_cell_offsets_cached_ = nullptr; }
    if (d_cell_data_cached_)    { cudaFree(reinterpret_cast<void*>(d_cell_data_cached_));    d_cell_data_cached_ = nullptr; }
    rx_count_cached_ = 0; cell_data_size_cached_ = 0; cell_count_cached_ = 0;
    gas_handle_ = 0;
    built_ = false;
}

void OptiXSceneAccelerator::CleanupOptiX()
{
    if (sbt_.raygenRecord)       { cudaFree(reinterpret_cast<void*>(sbt_.raygenRecord)); }
    if (sbt_.missRecordBase)     { cudaFree(reinterpret_cast<void*>(sbt_.missRecordBase)); }
    if (sbt_.hitgroupRecordBase) { cudaFree(reinterpret_cast<void*>(sbt_.hitgroupRecordBase)); }
    std::memset(&sbt_, 0, sizeof(sbt_));

    if (optix_pipeline_)       { optixPipelineDestroy(optix_pipeline_);     optix_pipeline_ = nullptr; }
    if (raygen_pg_)            { optixProgramGroupDestroy(raygen_pg_);      raygen_pg_ = nullptr; }
    if (miss_pg_)              { optixProgramGroupDestroy(miss_pg_);        miss_pg_ = nullptr; }
    if (closest_hit_pg_)       { optixProgramGroupDestroy(closest_hit_pg_); closest_hit_pg_ = nullptr; }
    if (any_hit_pg_)           { optixProgramGroupDestroy(any_hit_pg_);     any_hit_pg_ = nullptr; }
    if (optix_module_)         { optixModuleDestroy(optix_module_);         optix_module_ = nullptr; }
    if (optix_context_)        { optixDeviceContextDestroy(optix_context_); optix_context_ = nullptr; }
}

// ── External: Rx grid CUDA kernel ──
extern "C" void LaunchRxGridCheckKernel(
    int num_segments,
    const double* d_starts, const double* d_ends,
    const float* d_rx_positions,
    const int* d_cell_offsets, const int* d_cell_data,
    double cell_size, double sphere_radius_sq,
    double ox, double oy, double oz,
    int nx, int ny, int nz, int cell_count,
    int* d_hit_counts, int* d_hit_buffer, int max_hits_per_seg,
    cudaStream_t stream);

// ── GPU Rx grid query ──

std::vector<std::vector<int>> OptiXSceneAccelerator::QueryRxHitsBatch(
    const std::vector<double>& seg_starts_flat,
    const std::vector<double>& seg_ends_flat,
    const RxGridQueryParams& grid) const
{
    int N = static_cast<int>(seg_starts_flat.size() / 3);
    std::vector<std::vector<int>> results(N);
    if (N == 0 || !grid.rx_positions) return results;

    std::lock_guard<std::mutex> lock(launch_mutex_);

    int NRx = grid.rx_count;
    int cell_count = grid.nx * grid.ny * grid.nz;
    int data_size = grid.flat_cell_data_size;

    // Rx grid GPU cache: upload once, reuse across depth levels
    float* d_rx_pos = d_rx_positions_cached_;
    int* d_cell_offsets = d_cell_offsets_cached_;
    int* d_cell_data = d_cell_data_cached_;

    if (!d_rx_pos || NRx != rx_count_cached_ || cell_count != cell_count_cached_) {
        // First upload or grid changed: upload Rx positions
        if (d_rx_pos) cudaFree(reinterpret_cast<void*>(d_rx_pos));
        std::vector<float> rx_flat(NRx * 3);
        for (int i = 0; i < NRx; ++i) {
            double x = grid.rx_positions[i].x;
            double y = grid.rx_positions[i].y;
            double z = grid.rx_positions[i].z;
            rx_flat[i * 3 + 0] = static_cast<float>(x);
            rx_flat[i * 3 + 1] = static_cast<float>(y);
            rx_flat[i * 3 + 2] = static_cast<float>(z);
        }
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_rx_pos), NRx * 3 * sizeof(float)));
        CUDA_CHECK(cudaMemcpy(d_rx_pos, rx_flat.data(), NRx * 3 * sizeof(float), cudaMemcpyHostToDevice));
        d_rx_positions_cached_ = d_rx_pos; rx_count_cached_ = NRx;

        // Upload cell offsets
        if (d_cell_offsets) cudaFree(reinterpret_cast<void*>(d_cell_offsets));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_cell_offsets), (cell_count + 1) * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(d_cell_offsets, grid.cell_offsets, (cell_count + 1) * sizeof(int), cudaMemcpyHostToDevice));
        d_cell_offsets_cached_ = d_cell_offsets;

        // Upload cell data
        if (d_cell_data) cudaFree(reinterpret_cast<void*>(d_cell_data));
        CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_cell_data), data_size * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(d_cell_data, grid.flat_cell_data, data_size * sizeof(int), cudaMemcpyHostToDevice));
        d_cell_data_cached_ = d_cell_data;
        cell_data_size_cached_ = data_size; cell_count_cached_ = cell_count;
    }

    // Upload segment data
    double* d_starts = nullptr;
    double* d_ends = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_starts), N * 3 * sizeof(double)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_ends), N * 3 * sizeof(double)));
    CUDA_CHECK(cudaMemcpy(d_starts, seg_starts_flat.data(), N * 3 * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_ends, seg_ends_flat.data(), N * 3 * sizeof(double), cudaMemcpyHostToDevice));

    // Output buffers
    int max_hits = grid.max_hits_per_seg;
    int* d_hit_counts = nullptr;
    int* d_hit_buffer = nullptr;
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_counts), N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&d_hit_buffer), N * max_hits * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_hit_counts, 0, N * sizeof(int)));

    double sphere_radius_sq = grid.sphere_radius * grid.sphere_radius;

    LaunchRxGridCheckKernel(
        N, d_starts, d_ends, d_rx_pos,
        d_cell_offsets, d_cell_data,
        grid.cell_size, sphere_radius_sq,
        grid.ox, grid.oy, grid.oz,
        grid.nx, grid.ny, grid.nz, cell_count,
        d_hit_counts, d_hit_buffer, max_hits,
        nullptr);

    CUDA_CHECK(cudaDeviceSynchronize());

    // Read back
    std::vector<int> hit_counts(N);
    std::vector<int> hit_buffer(N * max_hits);
    CUDA_CHECK(cudaMemcpy(hit_counts.data(), d_hit_counts, N * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(hit_buffer.data(), d_hit_buffer, N * max_hits * sizeof(int), cudaMemcpyDeviceToHost));

    for (int i = 0; i < N; ++i) {
        int cnt = hit_counts[i];
        if (cnt > max_hits) cnt = max_hits;
        results[i].reserve(cnt);
        int base = i * max_hits;
        for (int j = 0; j < cnt; ++j)
            results[i].push_back(hit_buffer[base + j]);
    }

    // Free per-query temp buffers (Rx grid data is cached, not freed)
    CUDA_CHECK(cudaFree(d_starts));
    CUDA_CHECK(cudaFree(d_ends));
    CUDA_CHECK(cudaFree(d_hit_counts));
    CUDA_CHECK(cudaFree(d_hit_buffer));

    return results;
}

} // namespace rt
