// v8 Phase 2: 种子Rx采样器 — 从Rx网格中选择用于精确搜索的代表性Rx
#pragma once

#include "../common/math/Vec3.h"
#include <unordered_map>
#include <vector>

namespace rt {

struct RxSeedSamplerConfig {
    int target_seed_count = 2000;        // 目标种子Rx数
    double uniform_stride_m = 0.5;       // 均匀采样空间步长 (m), 0=仅自适应
    bool enable_adaptive = true;         // 自适应加密 (几何复杂度高处)
    double complexity_radius_m = 2.0;    // 几何复杂度评估半径 (m)
    double complexity_threshold = 10;    // 复杂度阈值 (>此值则加密采样)
};

struct RxSeedSamplerResult {
    std::vector<int> seed_indices;       // 种子Rx在rx_grid中的索引
    std::vector<int> seed_to_nearest_seed; // 每个Rx的最近种子索引 (用于路径复用)
    int seed_count = 0;
    double avg_rx_per_seed = 0.0;
    int max_rx_per_seed = 0;
};

class RxSeedSampler {
public:
    /// <summary>
    /// 从Rx网格中选择种子Rx。结合均匀采样和自适应加密。
    /// </summary>
    /// <param name="rx_grid">全量Rx位置网格</param>
    /// <param name="rx_active_mask">活跃Rx掩码 (粗扫后)</param>
    /// <param name="active_faces">每个活跃Rx的可见面元 (用于复杂度评估)</param>
    /// <param name="config">采样配置</param>
    static RxSeedSamplerResult Sample(
        const std::vector<Point3>& rx_grid,
        const std::vector<bool>& rx_active_mask,
        const std::unordered_map<int, std::vector<int>>& active_faces,
        const RxSeedSamplerConfig& config = {});
};

} // namespace rt
