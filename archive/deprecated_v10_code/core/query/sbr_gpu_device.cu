// v8: SBR GPU Megakernel - end-to-end ray processing on GPU
// Rx distance: double precision (matches CPU result)

#include <optix.h>

struct GPUFaceRecord {
    unsigned int face_id, object_id;
    float normal_x, normal_y, normal_z;
    unsigned int propagation_flags;
    float surface_eps_r, surface_sigma;
};

struct WedgeGpuData {
    float center_x, center_y, center_z;
    float dir_x, dir_y, dir_z;
    float length, wedge_angle_deg;
    int positive_face_id, negative_face_id;
    float n0face_x, n0face_y, n0face_z;
};

struct SbrMegakernelParams {
    OptixTraversableHandle traversable;
    const float* ray_origins, *ray_dirs;
    float tmin, tmax;
    float tx_ox, tx_oy, tx_oz;
    const float* rx_positions;
    const int* rx_cell_offsets, *rx_cell_data;
    float rx_cell_size, rx_sphere_r;
    float rx_ox, rx_oy, rx_oz;
    int rx_nx, rx_ny, rx_nz, rx_count, rx_cell_count;
    const GPUFaceRecord* face_records;
    int face_count;
    float* g_out_power;
    int* g_out_hits;
    int N_rays, maxDepth, maxRefl, maxTrans, maxDiff;
    float pwrTh, normFactor, freqHz;
    unsigned int tx_seed;
    float originIgnoreDist;
    const WedgeGpuData* wedge_records;
    int wedge_count;
    float wedge_max_dist;
    int wedge_max_candidates;
};

extern "C" { __constant__ SbrMegakernelParams megakernel_params; }

// ── Math helpers ──
__device__ float3 gpu_cross(float3 a, float3 b) {
    return make_float3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}
__device__ float gpu_dot(float3 a, float3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
__device__ float gpu_len(float3 v) { return sqrtf(gpu_dot(v,v)); }
__device__ float3 gpu_norm(float3 v) {
    float l=gpu_len(v); if(l<1e-9f) return make_float3(1,0,0);
    return make_float3(v.x/l, v.y/l, v.z/l);
}
__device__ float3 gpu_reflect(float3 d, float3 n) {
    float t=gpu_dot(d,n); return make_float3(d.x-2*t*n.x, d.y-2*t*n.y, d.z-2*t*n.z);
}
__device__ float3 gpu_refract(float3 d, float3 n, float cosI, float n2) {
    float st2=(1-cosI*cosI)/(n2*n2); if(st2>=1) return gpu_reflect(d,n);
    float cT=sqrtf(1-st2), idn=-cosI;
    return gpu_norm(make_float3((d.x-idn*n.x)/n2-n.x*cT,(d.y-idn*n.y)/n2-n.y*cT,(d.z-idn*n.z)/n2-n.z*cT));
}
__device__ unsigned gpu_xorshift(unsigned& s) { s^=s<<13; s^=s>>17; s^=s<<5; return s; }
__device__ float gpu_rand(unsigned& s) { return gpu_xorshift(s)/4294967296.0f; }

// ── Fresnel (GPU, complex arithmetic inline) ──
__device__ float gpu_fresnel_power(float cosI, float epsR, float sigma, float fHz) {
    if(cosI<1e-4f) cosI=1e-4f;
    float om=6.283185307179586f*fHz;
    float ei=(om>0)?sigma/(om*8.854187817e-12f):0;
    float a=epsR-(1-cosI*cosI), b=-ei;
    float mag=sqrtf(a*a+b*b);
    float rs=sqrtf((mag+a)*0.5f), is_=(b>=0?1:-1)*sqrtf((mag-a)*0.5f);
    float dn_re=cosI+rs, dn_im=is_;
    float dn2=dn_re*dn_re+dn_im*dn_im;
    float rTE_re=((cosI-rs)*dn_re+(-is_)*dn_im)/dn2;
    float rTE_im=((-is_)*dn_re-(cosI-rs)*dn_im)/dn2;
    float eCr=epsR*cosI, eCi=-ei*cosI;
    dn_re=eCr+rs; dn_im=eCi+is_; dn2=dn_re*dn_re+dn_im*dn_im;
    float rTM_re=((eCr-rs)*dn_re+(eCi-is_)*dn_im)/dn2;
    float rTM_im=((eCi-is_)*dn_re-(eCr-rs)*dn_im)/dn2;
    return 0.5f*(rTE_re*rTE_re+rTE_im*rTE_im+rTM_re*rTM_re+rTM_im*rTM_im);
}

// ── Point-to-segment distance squared (double precision == CPU) ──
__device__ double gpu_pt_seg_d2(
    double px, double py, double pz,
    double ax, double ay, double az,
    double bx, double by, double bz)
{
    double abx=bx-ax, aby=by-ay, abz=bz-az;
    double apx=px-ax, apy=py-ay, apz=pz-az;
    double ab2=abx*abx+aby*aby+abz*abz;
    if(ab2<=0.0) return apx*apx+apy*apy+apz*apz;
    double t=(apx*abx+apy*aby+apz*abz)/ab2;
    if(t<=0.0) return apx*apx+apy*apy+apz*apz;
    if(t>=1.0){ double dx=px-bx,dy=py-by,dz=pz-bz; return dx*dx+dy*dy+dz*dz; }
    double cx=ax+t*abx, cy=ay+t*aby, cz=az+t*abz;
    double dx=px-cx, dy=py-cy, dz=pz-cz;
    return dx*dx+dy*dy+dz*dz;
}

// ── Inline Rx grid check (double precision, matches CPU exactly) ──
__device__ void gpu_rx_check(
    float ax, float ay, float az, float bx, float by, float bz,
    float ray_power, int /*ray_idx*/)
{
    const double r2 = (double)megakernel_params.rx_sphere_r * (double)megakernel_params.rx_sphere_r;
    const double cell_sz = megakernel_params.rx_cell_size;
    const double ox = megakernel_params.rx_ox, oy = megakernel_params.rx_oy, oz = megakernel_params.rx_oz;
    const int nx=megakernel_params.rx_nx, ny=megakernel_params.rx_ny, nz=megakernel_params.rx_nz;

    double dax=ax, day=ay, daz=az, dbx=bx, dby=by, dbz=bz;
    double dx=dbx-dax, dy=dby-day, dz=dbz-daz;
    double seg_len = sqrt(dx*dx+dy*dy+dz*dz);

    int ns=1;
    if(fabs(dx)>2.0*cell_sz||fabs(dy)>2.0*cell_sz||fabs(dz)>2.0*cell_sz) {
        double step = fmax((double)megakernel_params.rx_sphere_r, cell_sz*0.5);
        ns = max(2, (int)ceil(seg_len/step));
    }

    int lh=0, lbuf[64];
    for(int si=0; si<ns; ++si) {
        double t = (ns==1) ? 0.5 : (double)si/(double)(ns-1);
        double mx=dax+t*dx, my=day+t*dy, mz=daz+t*dz;
        int cx=(int)((mx-ox)/cell_sz), cy=(int)((my-oy)/cell_sz), cz=(int)((mz-oz)/cell_sz);

        for(int ddx=-1;ddx<=1;++ddx) for(int ddy=-1;ddy<=1;++ddy) for(int ddz=-1;ddz<=1;++ddz) {
            int cxi=cx+ddx, cyi=cy+ddy, czi=cz+ddz;
            if(cxi<0||cxi>=nx||cyi<0||cyi>=ny||czi<0||czi>=nz) continue;
            int ci=cxi*ny*nz+cyi*nz+czi;
            if(ci<0||ci>=megakernel_params.rx_cell_count) continue;
            int os=megakernel_params.rx_cell_offsets[ci];
            int oe=megakernel_params.rx_cell_offsets[ci+1];
            for(int oi=os; oi<oe; ++oi) {
                int rxi=megakernel_params.rx_cell_data[oi];
                double px=megakernel_params.rx_positions[rxi*3+0];
                double py=megakernel_params.rx_positions[rxi*3+1];
                double pz=megakernel_params.rx_positions[rxi*3+2];
                if(gpu_pt_seg_d2(px,py,pz, dax,day,daz, dbx,dby,dbz) <= r2) {
                    bool dup=false;
                    for(int k=0;k<lh;++k) if(lbuf[k]==rxi){dup=true;break;}
                    if(!dup && lh<64) {
                        lbuf[lh++]=rxi;
                        atomicAdd(megakernel_params.g_out_power+rxi, ray_power*megakernel_params.normFactor);
                        atomicAdd(megakernel_params.g_out_hits+rxi, 1);
                    }
                }
            }
        }
    }
}

// ── Megakernel raygen ──
extern "C" __global__ void __raygen__sbr_megakernel()
{
    unsigned idx = optixGetLaunchIndex().x;
    if(idx >= (unsigned)megakernel_params.N_rays) return;

    unsigned N = megakernel_params.N_rays;
    float phi_c = 3.14159265358979323846f*(3.0f-sqrtf(5.0f));
    float y = 1.0f-(idx/(float)(N-1))*2.0f;
    float r = sqrtf(fmaxf(0.0f, 1.0f-y*y));
    float th = phi_c*(float)idx;
    float3 dir = make_float3(cosf(th)*r, y, sinf(th)*r);

    float3 origin = make_float3(megakernel_params.tx_ox, megakernel_params.tx_oy, megakernel_params.tx_oz);
    float power=1.0f; int cr=0, ct=0, cd=0;
    unsigned rng = idx*2654435761u+1;

    for(int depth=0; depth<=megakernel_params.maxDepth && power>megakernel_params.pwrTh; ++depth) {
        unsigned p0=0xFFFFFFFFu,p1=0xFFFFFFFFu,p2=0x7F7FFFFFu,p3=0u,p4=0u,p5=0u;
        optixTrace(OPTIX_PAYLOAD_TYPE_DEFAULT, megakernel_params.traversable,
            origin, dir, megakernel_params.originIgnoreDist, 1e30f, 0.0f,
            OptixVisibilityMask(0xFF), OPTIX_RAY_FLAG_NONE, 0u,0u,0u, p0,p1,p2,p3,p4,p5);

        int fid=(int)p0; if(fid<0) break;
        float thit=__uint_as_float(p2);
        float3 hpos=make_float3(origin.x+thit*dir.x, origin.y+thit*dir.y, origin.z+thit*dir.z);
        float3 nrm=make_float3(__uint_as_float(p3),__uint_as_float(p4),__uint_as_float(p5));

        gpu_rx_check(origin.x,origin.y,origin.z, hpos.x,hpos.y,hpos.z, power, idx);

        if(fid>=megakernel_params.face_count) break;
        GPUFaceRecord face=megakernel_params.face_records[fid];
        bool refl=(face.propagation_flags&1)!=0;
        bool trans=(face.propagation_flags&2)!=0;

        float cosI=fabsf(gpu_dot(dir,nrm)); if(cosI<1e-4f) cosI=1e-4f;
        float refPwr=gpu_fresnel_power(cosI, face.surface_eps_r, face.surface_sigma, megakernel_params.freqHz);

        bool bounced=false;
        if(ct<megakernel_params.maxTrans && trans) {
            if(gpu_rand(rng)>=refPwr) {
                float n2=sqrtf(fmaxf(1.0f, face.surface_eps_r));
                dir=gpu_refract(dir,nrm,cosI,n2);
                origin=make_float3(hpos.x+dir.x*0.001f, hpos.y+dir.y*0.001f, hpos.z+dir.z*0.001f);
                power*=(1.0f-refPwr); ct++; bounced=true;
            }
        }
        if(!bounced && cr<megakernel_params.maxRefl && refl) {
            dir=gpu_reflect(dir,nrm);
            origin=make_float3(hpos.x+dir.x*0.001f, hpos.y+dir.y*0.001f, hpos.z+dir.z*0.001f);
            power*=refPwr; cr++;
        } else if(!bounced) break;
    }
}

// ── Closest-hit + Miss ──
extern "C" __global__ void __closesthit__sbr_ch() {
    unsigned pi=optixGetPrimitiveIndex();
    float ht=optixGetRayTmax();
    if(ht<__uint_as_float(optixGetPayload_2())) {
        const GPUFaceRecord& rec=megakernel_params.face_records[pi];
        optixSetPayload_0(rec.face_id); optixSetPayload_1(rec.object_id);
        optixSetPayload_2(__float_as_uint(ht));
        optixSetPayload_3(__float_as_uint(rec.normal_x));
        optixSetPayload_4(__float_as_uint(rec.normal_y));
        optixSetPayload_5(__float_as_uint(rec.normal_z));
    }
}
extern "C" __global__ void __miss__sbr_ms() {}
