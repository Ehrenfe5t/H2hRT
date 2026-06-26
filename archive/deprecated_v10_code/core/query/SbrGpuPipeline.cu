// v8: SbrGpuPipeline CUDA side — OptiX setup + kernel launch (no project headers)
// Note: OPTIX_STUBS_INCLUDE_IMPL is defined in OptiXSceneAccelerator.cu

#include "SbrGpuPipeline.h"

#include <optix.h>
#include <optix_stubs.h>
#include <optix_function_table.h>
#include <optix_stack_size.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <stdexcept>
#include <vector>

#define CU_CHK(call) do { cudaError_t e = call; if (e != cudaSuccess) { \
    std::fprintf(stderr,"[SbrGPU] CUDA err %s:%d: %s\n",__FILE__,__LINE__,cudaGetErrorString(e)); \
    throw std::runtime_error(cudaGetErrorString(e)); } } while(0)

#define OX_CHK(call) do { OptixResult r = call; if (r != OPTIX_SUCCESS) { \
    std::fprintf(stderr,"[SbrGPU] OptiX err %s:%d: %d\n",__FILE__,__LINE__,(int)r); \
    throw std::runtime_error("OptiX error"); } } while(0)

namespace rt {

void SbrGpuPipeline::UploadGeometryData(const GpuVert* verts, const unsigned int* inds,
                                         const GpuFaceRecord* faces, int nf) {
    nt_ = nf; nv_ = nf * 3;
    CU_CHK(cudaMalloc((void**)&d_v_, nv_ * sizeof(GpuVert)));
    CU_CHK(cudaMemcpy(d_v_, verts, nv_*sizeof(GpuVert), cudaMemcpyHostToDevice));
    CU_CHK(cudaMalloc((void**)&d_i_, nv_ * sizeof(unsigned int)));
    CU_CHK(cudaMemcpy(d_i_, inds, nv_*sizeof(unsigned int), cudaMemcpyHostToDevice));
    CU_CHK(cudaMalloc((void**)&d_f_, nf * sizeof(GpuFaceRecord)));
    CU_CHK(cudaMemcpy(d_f_, faces, nf*sizeof(GpuFaceRecord), cudaMemcpyHostToDevice));
}

void SbrGpuPipeline::InitOptiX() {
    OX_CHK(optixInit());
    CreateContext();
    CreateModule();
    CreatePipeline();
    BuildSbt();
}

void SbrGpuPipeline::CreateContext() {
    OptixDeviceContextOptions o = {};
    o.logCallbackFunction = [](unsigned int lv, const char* t, const char* m, void*) {
        if (lv >= 4) return; std::fprintf(stderr,"[SbrGPU][%s] %s\n",t,m);
    };
    o.logCallbackLevel = 3;
    OX_CHK(optixDeviceContextCreate(nullptr, &o, &ctx_));
}

void SbrGpuPipeline::CreateModule() {
    const char* ptx_path = "x64/Release/sbr_gpu_device.ptx";
    std::string ptx;
    FILE* f = std::fopen(ptx_path, "rb");
    if (!f) f = std::fopen("sbr_gpu_device.ptx", "rb");
    if (!f) throw std::runtime_error("[SbrGPU] Cannot open sbr_gpu_device.ptx");
    std::fseek(f, 0, SEEK_END); long sz = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    ptx.resize(sz + 1, '\0'); std::fread(&ptx[0], 1, sz, f); std::fclose(f);

    OptixModuleCompileOptions mco = {};
    mco.maxRegisterCount = OPTIX_COMPILE_DEFAULT_MAX_REGISTER_COUNT;
    mco.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
    mco.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;

    OptixPipelineCompileOptions pco = {};
    pco.usesMotionBlur = false;
    pco.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pco.numPayloadValues = 6;
    pco.numAttributeValues = 2;
    pco.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pco.pipelineLaunchParamsVariableName = "megakernel_params";

    char log[4096]; size_t logSz = sizeof(log);
    OptixResult res = optixModuleCreate(ctx_, &mco, &pco, ptx.c_str(), ptx.size(), log, &logSz, &mod_);
    if (res != OPTIX_SUCCESS) {
        std::fprintf(stderr, "[SbrGPU] Module create failed:\n%s\n", log);
        throw std::runtime_error("OptiX module failed");
    }
}

void SbrGpuPipeline::CreatePipeline() {
    OptixProgramGroupOptions pgo = {};
    OptixProgramGroupDesc pgd = {};
    char log[4096]; size_t logSz;

    std::memset(&pgd, 0, sizeof(pgd));
    pgd.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;
    pgd.raygen.module = mod_;
    pgd.raygen.entryFunctionName = "__raygen__sbr_megakernel";
    logSz = sizeof(log);
    OX_CHK(optixProgramGroupCreate(ctx_, &pgd, 1, &pgo, log, &logSz, &rg_pg_));

    std::memset(&pgd, 0, sizeof(pgd));
    pgd.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;
    pgd.miss.module = mod_;
    pgd.miss.entryFunctionName = "__miss__sbr_ms";
    logSz = sizeof(log);
    OX_CHK(optixProgramGroupCreate(ctx_, &pgd, 1, &pgo, log, &logSz, &ms_pg_));

    std::memset(&pgd, 0, sizeof(pgd));
    pgd.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;
    pgd.hitgroup.moduleCH = mod_;
    pgd.hitgroup.entryFunctionNameCH = "__closesthit__sbr_ch";
    logSz = sizeof(log);
    OX_CHK(optixProgramGroupCreate(ctx_, &pgd, 1, &pgo, log, &logSz, &hg_pg_));

    OptixProgramGroup pgs[] = { rg_pg_, ms_pg_, hg_pg_ };
    int nPg = 3;

    OptixPipelineCompileOptions pco = {};
    pco.usesMotionBlur = false;
    pco.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
    pco.numPayloadValues = 6;
    pco.numAttributeValues = 2;
    pco.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pco.pipelineLaunchParamsVariableName = "megakernel_params";

    OptixPipelineLinkOptions plo = {};
    plo.maxTraceDepth = 1;

    logSz = sizeof(log);
    OX_CHK(optixPipelineCreate(ctx_, &pco, &plo, pgs, nPg, log, &logSz, &pipe_));

    OptixStackSizes ss = {};
    for (int i = 0; i < nPg; ++i) OX_CHK(optixUtilAccumulateStackSizes(pgs[i], &ss, pipe_));
    unsigned int dcc=0,ddc=0,cont=0;
    OX_CHK(optixUtilComputeStackSizes(&ss, 1, 0, 0, &dcc, &ddc, &cont));
    OX_CHK(optixPipelineSetStackSize(pipe_, dcc, ddc, cont, 1));
}

void SbrGpuPipeline::BuildSbt() {
    struct RgRec { char hdr[OPTIX_SBT_RECORD_HEADER_SIZE]; };
    RgRec rg; OX_CHK(optixSbtRecordPackHeader(rg_pg_, &rg));
    CU_CHK(cudaMalloc((void**)&sbt_.raygenRecord, sizeof(RgRec)));
    CU_CHK(cudaMemcpy((void*)sbt_.raygenRecord, &rg, sizeof(RgRec), cudaMemcpyHostToDevice));

    struct MsRec { char hdr[OPTIX_SBT_RECORD_HEADER_SIZE]; };
    MsRec ms; OX_CHK(optixSbtRecordPackHeader(ms_pg_, &ms));
    CU_CHK(cudaMalloc((void**)&sbt_.missRecordBase, sizeof(MsRec)));
    CU_CHK(cudaMemcpy((void*)sbt_.missRecordBase, &ms, sizeof(MsRec), cudaMemcpyHostToDevice));
    sbt_.missRecordStrideInBytes = sizeof(MsRec); sbt_.missRecordCount = 1;

    struct HgRec { char hdr[OPTIX_SBT_RECORD_HEADER_SIZE]; };
    HgRec hg; OX_CHK(optixSbtRecordPackHeader(hg_pg_, &hg));
    CU_CHK(cudaMalloc((void**)&sbt_.hitgroupRecordBase, sizeof(HgRec)));
    CU_CHK(cudaMemcpy((void*)sbt_.hitgroupRecordBase, &hg, sizeof(HgRec), cudaMemcpyHostToDevice));
    sbt_.hitgroupRecordStrideInBytes = sizeof(HgRec); sbt_.hitgroupRecordCount = 1;
}

void SbrGpuPipeline::BuildGAS() {
    OptixBuildInput bi = {}; bi.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;
    CUdeviceptr dvp = (CUdeviceptr)d_v_, dip = (CUdeviceptr)d_i_;
    bi.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
    bi.triangleArray.vertexStrideInBytes = sizeof(GpuVert);
    bi.triangleArray.numVertices = nv_;
    bi.triangleArray.vertexBuffers = &dvp;
    bi.triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
    bi.triangleArray.indexStrideInBytes = 3*sizeof(unsigned int);
    bi.triangleArray.numIndexTriplets = nt_;
    bi.triangleArray.indexBuffer = dip;
    unsigned int flg[1] = { OPTIX_GEOMETRY_FLAG_NONE };
    bi.triangleArray.flags = flg; bi.triangleArray.numSbtRecords = 1;

    OptixAccelBuildOptions abo = {};
    abo.buildFlags = OPTIX_BUILD_FLAG_PREFER_FAST_TRACE;
    abo.operation = OPTIX_BUILD_OPERATION_BUILD;

    OptixAccelBufferSizes abs;
    OX_CHK(optixAccelComputeMemoryUsage(ctx_, &abo, &bi, 1, &abs));

    CUdeviceptr d_tmp;
    CU_CHK(cudaMalloc((void**)&d_tmp, abs.tempSizeInBytes));
    CU_CHK(cudaMalloc((void**)&d_gas_, abs.outputSizeInBytes));

    OX_CHK(optixAccelBuild(ctx_, nullptr, &abo, &bi, 1, d_tmp, abs.tempSizeInBytes,
        d_gas_, abs.outputSizeInBytes, &gas_, nullptr, 0));
    CU_CHK(cudaFree((void*)d_tmp));
}

void SbrGpuPipeline::UploadRxData(
    const std::vector<float>& rx, const std::vector<int>& offs,
    const std::vector<int>& data, float cs, float sr,
    float ox, float oy, float oz, int nx, int ny, int nz)
{
    rx_cs_=cs; rx_sr_=sr; rx_ox_=ox; rx_oy_=oy; rx_oz_=oz;
    rx_nx_=nx; rx_ny_=ny; rx_nz_=nz;
    rx_cnt_=(int)rx.size()/3; rx_cc_=nx*ny*nz;

    int nRx = (int)rx.size() / 3;
    CU_CHK(cudaMalloc((void**)&d_rx_, nRx * 3 * sizeof(float)));
    CU_CHK(cudaMemcpy(d_rx_, rx.data(), nRx*3*sizeof(float), cudaMemcpyHostToDevice));
    CU_CHK(cudaMalloc((void**)&d_off_, (int)offs.size() * sizeof(int)));
    CU_CHK(cudaMemcpy(d_off_, offs.data(), offs.size()*sizeof(int), cudaMemcpyHostToDevice));
    CU_CHK(cudaMalloc((void**)&d_dat_, (int)data.size() * sizeof(int)));
    CU_CHK(cudaMemcpy(d_dat_, data.data(), data.size()*sizeof(int), cudaMemcpyHostToDevice));
}

void SbrGpuPipeline::Launch(int N_rays, float txx, float txy, float txz,
    std::vector<float>& outPwr, std::vector<int>& outHits)
{
    int NRx = (int)outPwr.size();
    if (!d_pow_) { CU_CHK(cudaMalloc((void**)&d_pow_, NRx * sizeof(float))); }
    if (!d_hit_) { CU_CHK(cudaMalloc((void**)&d_hit_, NRx * sizeof(int))); }
    CU_CHK(cudaMemset(d_pow_, 0, NRx * sizeof(float)));
    CU_CHK(cudaMemset(d_hit_, 0, NRx * sizeof(int)));

    SbrGpuLaunchParams lp = {};
    lp.traversable = gas_;
    lp.tmin = orig_ign_; lp.tmax = 1e30f;
    lp.tx_ox = txx; lp.tx_oy = txy; lp.tx_oz = txz;
    lp.face_records = d_f_; lp.face_count = nt_;
    lp.rx_positions = d_rx_; lp.rx_cell_offsets = d_off_; lp.rx_cell_data = d_dat_;
    lp.rx_cell_size = rx_cs_; lp.rx_sphere_r = rx_sr_;
    lp.rx_ox = rx_ox_; lp.rx_oy = rx_oy_; lp.rx_oz = rx_oz_;
    lp.rx_nx = rx_nx_; lp.rx_ny = rx_ny_; lp.rx_nz = rx_nz_;
    lp.rx_count = rx_cnt_; lp.rx_cell_count = rx_cc_;
    lp.g_out_power = d_pow_; lp.g_out_hits = d_hit_;
    lp.N_rays = N_rays;
    lp.maxDepth = max_d_; lp.maxRefl = max_r_;
    lp.maxTrans = max_t_; lp.maxDiff = 0;
    lp.pwrTh = pwr_th_; lp.normFactor = 1.0f / (float)N_rays;
    lp.freqHz = freq_hz_;
    lp.originIgnoreDist = orig_ign_;
    lp.tx_seed = 1234567u;
    lp.wedge_records = nullptr; lp.wedge_count = 0;

    CUdeviceptr d_params;
    CU_CHK(cudaMalloc((void**)&d_params, sizeof(SbrGpuLaunchParams)));
    CU_CHK(cudaMemcpy((void*)d_params, &lp, sizeof(SbrGpuLaunchParams), cudaMemcpyHostToDevice));

    OX_CHK(optixLaunch(pipe_, nullptr, d_params, sizeof(SbrGpuLaunchParams), &sbt_,
        (unsigned)N_rays, 1u, 1u));
    CU_CHK(cudaDeviceSynchronize());

    CU_CHK(cudaMemcpy(outPwr.data(), d_pow_, NRx*sizeof(float), cudaMemcpyDeviceToHost));
    CU_CHK(cudaMemcpy(outHits.data(), d_hit_, NRx*sizeof(int), cudaMemcpyDeviceToHost));

    CU_CHK(cudaFree((void*)d_params));
}

bool SbrGpuPipeline::Run(std::vector<float>& out_power, std::vector<int>& out_hits,
    int N_rays, const std::vector<float>& rx_flat, const std::vector<int>& rx_cell_offsets,
    const std::vector<int>& rx_cell_data, float rx_cell_size, float rx_sphere_r,
    float rx_ox, float rx_oy, float rx_oz, int rx_nx, int rx_ny, int rx_nz,
    float tx_x, float tx_y, float tx_z)
{
    UploadRxData(rx_flat, rx_cell_offsets, rx_cell_data, rx_cell_size, rx_sphere_r,
                  rx_ox, rx_oy, rx_oz, rx_nx, rx_ny, rx_nz);
    Launch(N_rays, tx_x, tx_y, tx_z, out_power, out_hits);
    return true;
}

void SbrGpuPipeline::Cleanup() {
    if (d_v_) { cudaFree((void*)d_v_); d_v_=nullptr; }
    if (d_i_) { cudaFree((void*)d_i_); d_i_=nullptr; }
    if (d_f_) { cudaFree((void*)d_f_); d_f_=nullptr; }
    if (d_rx_) { cudaFree((void*)d_rx_); d_rx_=nullptr; }
    if (d_off_) { cudaFree((void*)d_off_); d_off_=nullptr; }
    if (d_dat_) { cudaFree((void*)d_dat_); d_dat_=nullptr; }
    if (d_pow_) { cudaFree((void*)d_pow_); d_pow_=nullptr; }
    if (d_hit_) { cudaFree((void*)d_hit_); d_hit_=nullptr; }
    if (d_gas_) { cudaFree((void*)d_gas_); d_gas_=0; }
    if (sbt_.raygenRecord) { cudaFree((void*)sbt_.raygenRecord); }
    if (sbt_.missRecordBase) { cudaFree((void*)sbt_.missRecordBase); }
    if (sbt_.hitgroupRecordBase) { cudaFree((void*)sbt_.hitgroupRecordBase); }
    std::memset(&sbt_, 0, sizeof(sbt_));
    if (pipe_) { optixPipelineDestroy(pipe_); pipe_=nullptr; }
    if (rg_pg_) { optixProgramGroupDestroy(rg_pg_); rg_pg_=nullptr; }
    if (ms_pg_) { optixProgramGroupDestroy(ms_pg_); ms_pg_=nullptr; }
    if (hg_pg_) { optixProgramGroupDestroy(hg_pg_); hg_pg_=nullptr; }
    if (mod_) { optixModuleDestroy(mod_); mod_=nullptr; }
    if (ctx_) { optixDeviceContextDestroy(ctx_); ctx_=nullptr; }
}

} // namespace rt
