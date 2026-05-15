// v8 Phase 2: RxSeedSampler 实现
#include "RxSeedSampler.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace rt {

RxSeedSamplerResult RxSeedSampler::Sample(
    const std::vector<Point3>& rx_grid,
    const std::vector<bool>& rx_active_mask,
    const std::unordered_map<int, std::vector<int>>& active_faces,
    const RxSeedSamplerConfig& config)
{
    RxSeedSamplerResult result;
    const int M = static_cast<int>(rx_grid.size());
    if (M == 0) return result;

    std::set<int> seedSet;

    // ── Phase A: 均匀采样 ──
    if (config.uniform_stride_m > 0.0) {
        double stride = config.uniform_stride_m;
        for (int i = 0; i < M; ++i) {
            if (!rx_active_mask.empty() && i < static_cast<int>(rx_active_mask.size()) && !rx_active_mask[i])
                continue;
            const Point3& p = rx_grid[i];
            // 将 Rx 位置量化到 stride 网格
            int gx = static_cast<int>(std::floor(p.x / stride));
            int gy = static_cast<int>(std::floor(p.y / stride));
            int gz = static_cast<int>(std::floor(p.z / stride));
            // 取每个网格单元的第一个 Rx 作为种子
            uint64_t cellId = (uint64_t(gx & 0x1FFFF) << 42)
                            | (uint64_t(gy & 0x1FFFF) << 21)
                            | uint64_t(gz & 0x1FFFF);
            // 使用哈希表判断是否已选该 cell
            static std::unordered_map<uint64_t, int> cellFirst; // thread-local approach needed for parallel
            // Simplified: just stride-skip
            if (gx % std::max(1, static_cast<int>(stride)) == 0 &&
                gy % std::max(1, static_cast<int>(stride)) == 0 &&
                gz % std::max(1, static_cast<int>(stride)) == 0) {
                seedSet.insert(i);
            }
        }
    }

    // ── Phase B: 自适应加密 (几何复杂度高处) ──
    if (config.enable_adaptive && !active_faces.empty()) {
        for (int i = 0; i < M; ++i) {
            if (!rx_active_mask.empty() && i < static_cast<int>(rx_active_mask.size()) && !rx_active_mask[i])
                continue;
            auto it = active_faces.find(i);
            if (it == active_faces.end()) continue;
            int faceCount = static_cast<int>(it->second.size());
            if (faceCount > static_cast<int>(config.complexity_threshold)) {
                // 高复杂度 Rx 区域: 加密种子
                seedSet.insert(i);
                // 邻近 Rx 也加密
                for (int j = std::max(0, i - 10); j < std::min(M, i + 10); ++j) {
                    if (j != i && seedSet.count(j) == 0) {
                        double dx = rx_grid[i].x - rx_grid[j].x;
                        double dy = rx_grid[i].y - rx_grid[j].y;
                        double dz = rx_grid[i].z - rx_grid[j].z;
                        if (std::sqrt(dx*dx + dy*dy + dz*dz) < config.complexity_radius_m) {
                            seedSet.insert(j);
                        }
                    }
                }
            }
        }
    }

    // ── 截断到目标数量 ──
    result.seed_indices.assign(seedSet.begin(), seedSet.end());
    if (static_cast<int>(result.seed_indices.size()) > config.target_seed_count) {
        // 均匀下采样
        int step = static_cast<int>(result.seed_indices.size()) / config.target_seed_count;
        std::vector<int> downsampled;
        for (size_t i = 0; i < result.seed_indices.size(); i += std::max(1, step))
            downsampled.push_back(result.seed_indices[i]);
        result.seed_indices = downsampled;
    }
    result.seed_count = static_cast<int>(result.seed_indices.size());

    // ── 最近种子映射 ──
    result.seed_to_nearest_seed.resize(M, -1);
    for (int i = 0; i < M; ++i) {
        double bestDist = 1e30;
        int bestSeed = -1;
        for (int si : result.seed_indices) {
            double dx = rx_grid[i].x - rx_grid[si].x;
            double dy = rx_grid[i].y - rx_grid[si].y;
            double dz = rx_grid[i].z - rx_grid[si].z;
            double d = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (d < bestDist) { bestDist = d; bestSeed = si; }
        }
        result.seed_to_nearest_seed[i] = bestSeed;
    }

    // 统计每种子覆盖的Rx数
    std::unordered_map<int, int> perSeedCount;
    for (int si : result.seed_to_nearest_seed)
        if (si >= 0) perSeedCount[si]++;
    if (!perSeedCount.empty()) {
        result.avg_rx_per_seed = static_cast<double>(M) / result.seed_count;
        for (auto& kv : perSeedCount)
            result.max_rx_per_seed = std::max(result.max_rx_per_seed, kv.second);
    }

    return result;
}

} // namespace rt
