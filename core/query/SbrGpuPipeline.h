// v8: SBR GPU Megakernel host pipeline
#pragma once

#include <cuda_runtime.h>
#include <optix_types.h>
#include <vector>
#include <cstdio>

namespace rt {

struct Scene;
struct AppConfig;

// GPU vertex
struct GpuVert { float x, y, z; };

// GPU-side face record (must match sbr_gpu_device.cu)
struct GpuFaceRecord {
    unsigned int face_id;
    unsigned int object_id;
    float normal_x, normal_y, normal_z;
    unsigned int propagation_flags;
    float surface_eps_r;
    float surface_sigma;
};

// GPU-side wedge data (for later diffraction support)
struct alignas(16) WedgeGpuData {
    float center_x, center_y, center_z;
    float dir_x, dir_y, dir_z;
    float length, wedge_angle_deg;
    int positive_face_id, negative_face_id;
    float n0face_x, n0face_y, n0face_z;
};

// Launch params (mirrors __constant__ struct in device code)
struct SbrGpuLaunchParams {
    OptixTraversableHandle traversable;
    const float* ray_origins;
    const float* ray_dirs;
    float tmin, tmax;
    float tx_ox, tx_oy, tx_oz;
    const float* rx_positions;
    const int* rx_cell_offsets;
    const int* rx_cell_data;
    float rx_cell_size, rx_sphere_r;
    float rx_ox, rx_oy, rx_oz;
    int   rx_nx, rx_ny, rx_nz, rx_count, rx_cell_count;
    const GpuFaceRecord* face_records;
    int face_count;
    float* g_out_power;
    int*   g_out_hits;
    int N_rays;
    int maxDepth, maxRefl, maxTrans, maxDiff;
    float pwrTh, normFactor, freqHz;
    unsigned int tx_seed;
    float originIgnoreDist;
    const WedgeGpuData* wedge_records;
    int wedge_count;
    float wedge_max_dist;
    int wedge_max_candidates;
};

class SbrGpuPipeline {
public:
    SbrGpuPipeline(const Scene& scene, const AppConfig& config);
    ~SbrGpuPipeline();

    bool Run(std::vector<float>& out_power, std::vector<int>& out_hits,
             int N_rays,
             const std::vector<float>& rx_flat,
             const std::vector<int>& rx_cell_offsets,
             const std::vector<int>& rx_cell_data,
             float rx_cell_size, float rx_sphere_r,
             float rx_ox, float rx_oy, float rx_oz,
             int rx_nx, int rx_ny, int rx_nz,
             float tx_x, float tx_y, float tx_z);

private:
    void InitOptiX();
    void CreateContext();
    void CreateModule();
    void CreatePipeline();
    void BuildSbt();
    void BuildGAS();
    // UploadGeometryData: called from .cpp (MSVC-compiled, accesses Scene)
    void UploadGeometryData(const GpuVert* verts, const unsigned int* inds,
                            const GpuFaceRecord* faces, int nf);
    void UploadRxData(const std::vector<float>& rx, const std::vector<int>& offs,
                      const std::vector<int>& data, float cs, float sr,
                      float ox, float oy, float oz, int nx, int ny, int nz);
    void Launch(int N_rays, float txx, float txy, float txz,
                std::vector<float>& outPwr, std::vector<int>& outHits);
    void Cleanup();

    int dev_id_ = 0;
    int max_d_, max_r_, max_t_, max_x_;  // depth, refl, trans, diff
    float pwr_th_, freq_hz_, orig_ign_;

    // GPU geometry
    GpuVert* d_v_ = nullptr;
    unsigned int* d_i_ = nullptr;
    GpuFaceRecord* d_f_ = nullptr;
    int nv_ = 0, nt_ = 0;

    // GPU Rx data + stored params for Launch
    float* d_rx_ = nullptr; int* d_off_ = nullptr; int* d_dat_ = nullptr;
    float rx_cs_, rx_sr_, rx_ox_, rx_oy_, rx_oz_;
    int rx_nx_, rx_ny_, rx_nz_, rx_cnt_, rx_cc_;

    // GPU output
    float* d_pow_ = nullptr;
    int* d_hit_ = nullptr;

    // OptiX
    OptixDeviceContext ctx_ = nullptr;
    OptixModule mod_ = nullptr;
    OptixProgramGroup rg_pg_ = nullptr, ms_pg_ = nullptr, hg_pg_ = nullptr;
    OptixPipeline pipe_ = nullptr;
    OptixShaderBindingTable sbt_ = {};
    OptixTraversableHandle gas_ = 0;
    CUdeviceptr d_gas_ = 0;
};

} // namespace rt
