// v8 Phase 4: GPU+CUDA+OptiX geometry acceleration
// RTX 3060 12GB (CC 8.6) + CUDA 12.6 + OptiX 7.7
#pragma once

#include "ISceneAccelerator.h"

#include <cuda_runtime.h>
#include <optix_types.h>

#include <vector>
#include <memory>
#include <string>
#include <mutex>

namespace rt {
struct Scene;
struct AppConfig;
}

namespace rt {

// ── GPU 端数据结构 (CPU 端镜像, 用于上传) ──

struct GPUVertex {
    float x, y, z;
};

struct GPUFaceRecord {
    unsigned int face_id;
    unsigned int object_id;
    float normal_x, normal_y, normal_z;
    unsigned int propagation_flags;
};

// SBT record data (per-hitgroup instance)
struct SbtRecordData {
    int material_index;
};

// hitgroup SBT record (header + data)
struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) HitgroupSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    SbtRecordData data;
};

// raygen SBT record
struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) RaygenSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
};

// miss SBT record
struct __align__(OPTIX_SBT_RECORD_ALIGNMENT) MissSbtRecord {
    char header[OPTIX_SBT_RECORD_HEADER_SIZE];
};

// ── Unified launch params — host and device share via __constant__ ──

struct UnifiedLaunchParams {
    OptixTraversableHandle traversable;

    // Ray data (used by both closest-hit and occlusion queries)
    const float* ray_origins;    // [N*3]
    const float* ray_dirs;       // [N*3]
    float tmin;
    float tmax;

    // Closest-hit outputs
    int* hit_flags;              // [N]
    int* hit_face_ids;           // [N]
    int* hit_object_ids;         // [N]
    float* hit_distances;        // [N]
    float* hit_positions;        // [N*3]
    float* hit_normals;          // [N*3]

    // Occlusion outputs
    int* occluded;               // [N]

    int ray_count;
    int query_type;              // 0 = closest-hit, 1 = occlusion
    int ignored_face_id;         // face to ignore in hit detection
    int ignored_face_id2;        // second face to ignore
    const GPUFaceRecord* face_records;
};

/// <summary>
/// GPU OptiX 场景加速器 — 实现 ISceneAccelerator 接口。
/// 使用 RT Core 硬件加速 BVH 遍历和射线-三角求交。
/// </summary>
class OptiXSceneAccelerator : public ISceneAccelerator {
public:
    OptiXSceneAccelerator(const Scene& scene, const AppConfig& config);
    ~OptiXSceneAccelerator() override;

    // ── ISceneAccelerator 接口实现 ──

    FaceHit QueryClosestFaceHit(const Ray& ray, const FaceQueryContext& ctx) const override;
    FaceHit QueryClosestFaceHitFast(const Ray& ray, const FaceQueryContext& ctx) const override;
    std::vector<FaceHit> QueryAllFaceHits(const Ray& ray, const FaceQueryContext& ctx) const override;
    std::vector<FaceHit> QueryFaceHitsInRange(const Ray& ray, double tMin, double tMax, const FaceQueryContext& ctx) const override;
    bool IsOccluded(const Point3& start, const Point3& end, const VisibilityQueryContext& ctx) const override;

    // ── 批量查询 (GPU 优化) ──
    std::vector<FaceHit> QueryClosestFaceHitBatch(const std::vector<Ray>& rays, const FaceQueryContext& ctx) const override;
    std::vector<bool> IsOccludedBatch(const std::vector<Point3>& starts, const std::vector<Point3>& ends,
                                       const VisibilityQueryContext& ctx) const override;

    // ── v8 GPU: batch Rx grid query ──
    std::vector<std::vector<int>> QueryRxHitsBatch(
        const std::vector<double>& seg_starts_flat,
        const std::vector<double>& seg_ends_flat,
        const RxGridQueryParams& grid) const override;

    // ── 加速器元数据 ──
    std::string BackendName() const override { return "GPU_OptiX"; }
    bool SupportsBatchQuery() const override { return true; }
    size_t MaxBatchSize() const override { return max_batch_size_; }

    // ── 场景数据生命周期 ──
    void BuildFromScene(const Scene& scene) override;
    bool IsBuilt() const { return built_; }

private:
    // ── OptiX 初始化 ──
    void InitOptiX();
    void CreateContext();
    void CreateModule();
    void CreateProgramGroups();
    void CreatePipeline();
    void BuildSbt();

    // ── 场景数据上传 (由 .cpp 调用, .cu 实现) ──
    void UploadSceneGeometry(const Scene& scene);
    void UploadGeometryData(const float* vertices, const unsigned int* indices,
                            const GPUFaceRecord* face_records, int num_faces);
    void BuildAccel();

    // ── 清理 ──
    void CleanupGpuBuffers();
    void CleanupOptiX();

    // ── 辅助方法 ──
    void LaunchClosestHitQuery(const float* origins, const float* dirs, int rayCount,
                               float tMin, float tMax, int* hitFlags, int* hitFaceIds,
                               float* hitDistances, float* hitPositions, float* hitNormals,
                               int ignoredFaceId = -1, int ignoredFaceId2 = -1) const;
    void LaunchOcclusionQuery(const float* origins, const float* dirs, int rayCount,
                              float tMin, float tMax, int* occluded,
                              int ignoredFaceId = -1, int ignoredFaceId2 = -1) const;
    FaceHit BuildFaceHit(int idx, const int* hitFlags, const int* hitFaceIds,
                         const float* hitDistances, const float* hitPositions) const;

    // ── CUDA 工具 ──
    template<typename T>
    T* AllocDevice(size_t count) const;
    template<typename T>
    void CopyToDevice(T* dst, const T* src, size_t count) const;
    template<typename T>
    void CopyFromDevice(T* dst, const T* src, size_t count) const;

    // ── 场景引用与配置 ──
    const Scene& scene_;
    const AppConfig& config_;
    int device_id_ = 0;
    size_t max_batch_size_ = 1000000;
    bool built_ = false;

    // ── GPU 场景 buffer ──
    GPUVertex* d_vertices_ = nullptr;
    unsigned int* d_indices_ = nullptr;
    GPUFaceRecord* d_face_records_ = nullptr;
    int vertex_count_ = 0;
    int triangle_count_ = 0;

    // ── Rx grid GPU cache (upload once, reuse across depth levels) ──
    mutable float* d_rx_positions_cached_ = nullptr;
    mutable int* d_cell_offsets_cached_ = nullptr;
    mutable int* d_cell_data_cached_ = nullptr;
    mutable int rx_count_cached_ = 0;
    mutable int cell_data_size_cached_ = 0;
    mutable int cell_count_cached_ = 0;

    // ── OptiX 对象 ──
    OptixDeviceContext optix_context_ = nullptr;
    OptixModule optix_module_ = nullptr;
    OptixProgramGroup raygen_pg_ = nullptr;
    OptixProgramGroup miss_pg_ = nullptr;
    OptixProgramGroup closest_hit_pg_ = nullptr;
    OptixProgramGroup any_hit_pg_ = nullptr;
    OptixPipeline optix_pipeline_ = nullptr;
    OptixShaderBindingTable sbt_ = {};

    // ── OptiX BVH (GAS) ──
    OptixTraversableHandle gas_handle_ = 0;
    CUdeviceptr d_gas_output_ = 0;

    // ── 线程安全 ──
    mutable std::mutex launch_mutex_;
};

} // namespace rt
