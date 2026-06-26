// v8 GPU: batch segment-Rx collision detection kernel
// Replaces CPU RxHashGrid::CheckSegment with GPU parallel processing

#include <cuda_runtime.h>

extern "C" {

// ── 3D point-to-segment squared distance ──
__device__ double PointToSegmentDistSq(
    double px, double py, double pz,
    double ax, double ay, double az,
    double bx, double by, double bz)
{
    double abx = bx - ax, aby = by - ay, abz = bz - az;
    double apx = px - ax, apy = py - ay, apz = pz - az;
    double ab2 = abx * abx + aby * aby + abz * abz;
    if (ab2 <= 0.0) return apx * apx + apy * apy + apz * apz;
    double t = (apx * abx + apy * aby + apz * abz) / ab2;
    if (t <= 0.0) return apx * apx + apy * apy + apz * apz;
    if (t >= 1.0) {
        double dx = px - bx, dy = py - by, dz = pz - bz;
        return dx * dx + dy * dy + dz * dz;
    }
    double cx = ax + t * abx, cy = ay + t * aby, cz = az + t * abz;
    double dx = px - cx, dy = py - cy, dz = pz - cz;
    return dx * dx + dy * dy + dz * dz;
}

// ── Main kernel ──
// grid: 1D blocks, each block = 1 segment, blockDim = 32 threads (warp)
// Each thread processes different sample points along the segment

__global__ void RxGridCheckKernel(
    int num_segments,
    const double* seg_starts,      // [N*3]
    const double* seg_ends,        // [N*3]
    const float* rx_positions,     // [rxCount*3]
    const int* cell_offsets,       // [cell_count+1]
    const int* cell_data,          // [rxCount]
    double cell_size, double sphere_radius_sq,
    double ox, double oy, double oz,
    int nx, int ny, int nz,
    int cell_count,
    int* hit_counts,               // [N] output
    int* hit_buffer,               // [N * max_hits] output
    int max_hits_per_seg)
{
    int seg_idx = blockIdx.x;
    if (seg_idx >= num_segments) return;

    // Load segment data
    double ax = seg_starts[seg_idx * 3 + 0];
    double ay = seg_starts[seg_idx * 3 + 1];
    double az = seg_starts[seg_idx * 3 + 2];
    double bx = seg_ends[seg_idx * 3 + 0];
    double by = seg_ends[seg_idx * 3 + 1];
    double bz = seg_ends[seg_idx * 3 + 2];

    double dx = bx - ax, dy = by - ay, dz = bz - az;
    double seg_len = sqrt(dx * dx + dy * dy + dz * dz);

    // ── Determine sampling ──
    double sphere_r = sqrt(sphere_radius_sq);
    int n_samples = 1;
    bool spans_multi = false;
    if (fabs(dx) > 2.0 * cell_size || fabs(dy) > 2.0 * cell_size || fabs(dz) > 2.0 * cell_size) {
        spans_multi = true;
        double step = max(sphere_r, cell_size * 0.5);
        n_samples = max(2, static_cast<int>(ceil(seg_len / step)));
    }

    // ── Shared dedup buffer per warp ──
    // Simple approach: each thread handles one sample, avoids shared memory complexity
    int samples_per_thread = (n_samples + blockDim.x - 1) / blockDim.x;
    int sample_start = threadIdx.x * samples_per_thread;
    int sample_end = min(sample_start + samples_per_thread, n_samples);

    // ── Local hit buffer ──
    int local_hits = 0;
    int local_buffer[256];

    for (int si = sample_start; si < sample_end; ++si) {
        double t = (n_samples == 1) ? 0.5 : static_cast<double>(si) / static_cast<double>(n_samples - 1);
        double mx = ax + t * dx;
        double my = ay + t * dy;
        double mz = az + t * dz;

        // Determine cell index for this sample point
        int cx = static_cast<int>((mx - ox) / cell_size);
        int cy = static_cast<int>((my - oy) / cell_size);
        int cz = static_cast<int>((mz - oz) / cell_size);

        // Check 27-neighborhood
        for (int ddx = -1; ddx <= 1; ++ddx) {
            for (int ddy = -1; ddy <= 1; ++ddy) {
                for (int ddz = -1; ddz <= 1; ++ddz) {
                    int cxi = cx + ddx, cyi = cy + ddy, czi = cz + ddz;
                    if (cxi < 0 || cxi >= nx || cyi < 0 || cyi >= ny || czi < 0 || czi >= nz)
                        continue;

                    int cell_idx = cxi * ny * nz + cyi * nz + czi;
                    if (cell_idx < 0 || cell_idx >= cell_count) continue;

                    int off_start = cell_offsets[cell_idx];
                    int off_end = cell_offsets[cell_idx + 1];

                    for (int ci = off_start; ci < off_end; ++ci) {
                        int rxi = cell_data[ci];
                        double rx = static_cast<double>(rx_positions[rxi * 3 + 0]);
                        double ry = static_cast<double>(rx_positions[rxi * 3 + 1]);
                        double rz = static_cast<double>(rx_positions[rxi * 3 + 2]);

                        double d2 = PointToSegmentDistSq(rx, ry, rz, ax, ay, az, bx, by, bz);
                        if (d2 <= sphere_radius_sq) {
                            // Check local dedup within this thread
                            bool dup = false;
                            for (int k = 0; k < local_hits; ++k) {
                                if (local_buffer[k] == rxi) { dup = true; break; }
                            }
                            if (!dup && local_hits < 256) {
                                local_buffer[local_hits++] = rxi;
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Warp-level dedup and output ──
    __shared__ int warp_hits[32][128];
    __shared__ int warp_counts[32];

    warp_counts[threadIdx.x] = local_hits;
    for (int k = 0; k < local_hits && k < 128; ++k)
        warp_hits[threadIdx.x][k] = local_buffer[k];
    __syncthreads();

    if (threadIdx.x == 0) {
        int total = 0;
        int merged[512];

        for (int t = 0; t < blockDim.x; ++t) {
            for (int k = 0; k < warp_counts[t]; ++k) {
                int rxi = warp_hits[t][k];
                bool dup = false;
                for (int m = 0; m < total; ++m) {
                    if (merged[m] == rxi) { dup = true; break; }
                }
                if (!dup && total < 512) {
                    merged[total++] = rxi;
                }
            }
        }

        hit_counts[seg_idx] = total;
        int base = seg_idx * max_hits_per_seg;
        for (int m = 0; m < total && m < max_hits_per_seg; ++m) {
            hit_buffer[base + m] = merged[m];
        }
    }
}

// ── Host-side kernel launcher ──

void LaunchRxGridCheckKernel(
    int num_segments,
    const double* d_starts, const double* d_ends,
    const float* d_rx_positions,
    const int* d_cell_offsets, const int* d_cell_data,
    double cell_size, double sphere_radius_sq,
    double ox, double oy, double oz,
    int nx, int ny, int nz, int cell_count,
    int* d_hit_counts, int* d_hit_buffer, int max_hits_per_seg,
    cudaStream_t stream)
{
    int block_size = 32;  // one warp per block
    int grid_size = num_segments;

    RxGridCheckKernel<<<grid_size, block_size, 0, stream>>>(
        num_segments, d_starts, d_ends,
        d_rx_positions, d_cell_offsets, d_cell_data,
        cell_size, sphere_radius_sq,
        ox, oy, oz, nx, ny, nz, cell_count,
        d_hit_counts, d_hit_buffer, max_hits_per_seg);
}

} // extern "C"
