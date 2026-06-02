#pragma once

#include "../common/config/AppConfig.h"
#include "../common/material/MaterialDatabase.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"
#include "../query/SceneQuery.h"
#include <unordered_map>
#include <vector>

namespace rt {

// ── 原有 SBR coverage 模式 (保持兼容) ──

struct RxCoverageRecord {
    Point3 rx_position;
    int rx_index = -1;
    double total_power_linear = 0.0;
    double total_power_dBm = 0.0;
    int ray_hit_count = 0;
    std::vector<GeometricPath> paths;
};

struct SbrCoverageResult {
    bool succeeded = false;
    int total_rays = 0;
    int active_rx_count = 0;
    std::vector<RxCoverageRecord> rx_records;
    std::vector<std::string> trace_lines;
    // v9 Stage8: SBR diagnostics
    int total_bounces = 0;              // 总反射次数
    int total_transmissions = 0;        // 总透射次数
    int total_diffractions = 0;         // 总绕射次数
    int rays_below_threshold = 0;       // 功率低于阈值的射线数
    int rays_terminated_early = 0;      // 无交互提前终止的射线
    std::string convergence_notes;       // 收敛性说明
};

struct SbrContext {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const SceneQuery* scene_query = nullptr;
    const MaterialDatabase* material_db = nullptr;
    Point3 tx_point;
    std::vector<Point3> rx_grid;
    bool store_paths = false;
    double tx_power_dBm = 0.0;
};

// ── v8 Phase 2: SBR 粗扫模式 (面元可见性收集, 非功率累加) ──

struct SbrCoarseContext {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const SceneQuery* scene_query = nullptr;
    Point3 tx_point;
    std::vector<Point3> rx_grid;
    int coarse_ray_count = 50000;            // 粗扫射线数 (vs 200万 full SBR)
    int coarse_max_depth = 3;                // 粗扫深度 (仅收集低阶可见性)
    double coarse_rx_sphere_radius = 2.0;    // 放大接收球 (5-15x full SBR)
    bool expand_pvs = true;                  // 是否将 PVS 扩展到活跃集
};

struct SbrCoarseResult {
    bool succeeded = false;
    int total_coarse_rays = 0;
    int active_rx_count = 0;
    // Per-Rx 活跃面元/楔边 (稀疏: 仅存储活跃 Rx)
    std::unordered_map<int, std::vector<int>> rx_active_faces;
    std::unordered_map<int, std::vector<int>> rx_active_wedges;
    // 活跃 Rx 掩码 (true = 此 Rx 被至少一条粗扫射线命中)
    std::vector<bool> rx_active_mask;
    std::vector<std::string> trace_lines;
};

class SbrEngine {
public:
    // 原有: 标量功率 coverage (v8 GPU: wavefront 批量处理)
    SbrCoverageResult Run(const SbrContext& context) const;

    // v8 Phase 2: 粗扫 — 收集 Rx 活跃面元/楔边
    SbrCoarseResult RunCoarsePass(const SbrCoarseContext& context) const;

    // v10: 精确几何寻径 — 确定性射线分裂 (面元: R+T, 棱边: Keller锥)
    SbrCoverageResult RunPrecise(const SbrContext& context) const;

private:
    // v8 GPU wavefront: 按深度分波的批量射线处理
    SbrCoverageResult RunWavefront(const SbrContext& context) const;
    // v8 GPU megakernel: end-to-end single-launch SBR
    SbrCoverageResult RunMegakernel(const SbrContext& context) const;
};

} // namespace rt
