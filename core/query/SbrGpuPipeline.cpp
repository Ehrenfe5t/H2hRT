// v8: SbrGpuPipeline — C++ side (compiled by MSVC, accesses Scene internals)
#include "SbrGpuPipeline.h"
#include "../scene/Scene.h"
#include "../common/config/AppConfig.h"

#include <cuda_runtime.h>
#include <cstdio>
#include <cmath>
#include <stdexcept>

#define CU_CHK(call) do { cudaError_t e = call; if (e != cudaSuccess) { \
    std::fprintf(stderr,"[SbrGPU] CUDA err %s:%d: %s\n",__FILE__,__LINE__,cudaGetErrorString(e)); \
    throw std::runtime_error(cudaGetErrorString(e)); } } while(0)

namespace rt {

SbrGpuPipeline::SbrGpuPipeline(const Scene& scene, const AppConfig& config) {
    const auto& cfg = config.sbr;
    max_d_ = cfg.max_ray_depth; max_r_ = cfg.max_reflection_count;
    max_t_ = cfg.max_transmission_count; max_x_ = cfg.max_diffraction_count;
    pwr_th_ = (float)std::pow(10.0, cfg.ray_power_threshold_dB / 10.0);
    freq_hz_ = (float)config.em_solver.frequency_hz;
    orig_ign_ = (float)config.numeric_tolerance.self_hit_ignore_distance;
    dev_id_ = config.acceleration.gpu_device_id;

    CU_CHK(cudaSetDevice(dev_id_));
    CU_CHK(cudaFree(nullptr));

    // Upload scene geometry to GPU
    int nf = (int)scene.faces.size();
    nt_ = nf; nv_ = nf * 3;
    std::vector<GpuVert> verts(nv_);
    std::vector<unsigned int> inds(nv_);
    std::vector<GpuFaceRecord> faces(nf);
    for (int i = 0; i < nf; ++i) {
        const auto& f = scene.faces[i];
        int vi = i * 3;
        const auto& v0 = scene.vertices[f.vertex_index0];
        const auto& v1 = scene.vertices[f.vertex_index1];
        const auto& v2 = scene.vertices[f.vertex_index2];
        verts[vi+0] = {(float)v0.x,(float)v0.y,(float)v0.z};
        verts[vi+1] = {(float)v1.x,(float)v1.y,(float)v1.z};
        verts[vi+2] = {(float)v2.x,(float)v2.y,(float)v2.z};
        inds[vi+0] = vi; inds[vi+1] = vi+1; inds[vi+2] = vi+2;
        faces[i].face_id = (unsigned)f.face_id;
        faces[i].object_id = (unsigned)f.object_id;
        faces[i].normal_x = (float)f.normal.x;
        faces[i].normal_y = (float)f.normal.y;
        faces[i].normal_z = (float)f.normal.z;
        // Encode enabled flags into propagation_flags for GPU compatibility
        unsigned int flg = f.propagation_flags;
        if (f.reflection_enabled)   flg |= 1u;   // FacePropagationReflect
        if (f.transmission_enabled) flg |= 2u;   // FacePropagationTransmit
        faces[i].propagation_flags = flg;
        faces[i].surface_eps_r = (float)f.surface_eps_r;
        faces[i].surface_sigma = (float)f.surface_sigma;
    }
    UploadGeometryData(verts.data(), inds.data(), faces.data(), nf);

    InitOptiX();
    BuildGAS();
    std::fprintf(stdout, "[SbrGpuPipeline] Init OK: %d faces\n", nf);
}

SbrGpuPipeline::~SbrGpuPipeline() { Cleanup(); }

} // namespace rt
