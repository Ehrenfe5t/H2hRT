#pragma once

#include "../common/config/AppConfig.h"
#include "../common/material/MaterialDatabase.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"
#include "../query/SceneQuery.h"
#include <string>
#include <vector>

namespace rt {

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
    std::string trace_profile = "P2P-SBR";
    int total_rays = 0;
    int active_rx_count = 0;
    std::vector<RxCoverageRecord> rx_records;
    std::vector<std::string> trace_lines;
    int total_bounces = 0;
    int total_transmissions = 0;
    int total_diffractions = 0;
    int rays_below_threshold = 0;
    int rays_terminated_early = 0;
    long long generated_reflection_branches = 0;    // v10: generated reflection successor states
    long long generated_transmission_branches = 0;  // v10: generated transmission successor states
    long long rejected_tir_transmissions = 0;        // v10: transmission candidates rejected by TIR precheck
    long long pruned_power_branches = 0;             // v10: successor states below the SBR power threshold
    long long rx_paths_recorded = 0;                 // v10: paths kept after per-ray cap and signature dedup
    long long rx_paths_skipped_by_cap = 0;           // v10: path records skipped by max_paths_per_ray
    long long rx_paths_skipped_by_rx_cap = 0;        // v10: path records skipped by max_paths_per_rx
    long long rx_paths_deduplicated = 0;             // v10: duplicate path signatures suppressed
    int peak_active_rays = 0;                        // v10: maximum active wavefront size
    bool dynamic_rx_radius_enabled = false;          // v10: main-ray Rx hit radius followed ray-tube estimate
    double ray_tube_angle_rad = 0.0;                 // v10: effective angular spacing used for ray-tube radius
    double max_effective_rx_radius_m = 0.0;          // v10: maximum effective radius observed on main-ray Rx queries
    long long dynamic_rx_queries = 0;                // v10: main-ray dynamic Rx radius checks
    long long dynamic_rx_hits = 0;                   // v10: Rx hits returned by dynamic checks
    bool wedge_tube_coupling_enabled = false;        // v10: diffraction wedge candidates from ray-tube coupling
    long long wedge_tube_queries = 0;                // v10: main-ray segments checked against wedge tubes
    long long wedge_tube_candidates = 0;             // v10: wedge couplings accepted by segment-segment distance
    long long wedge_tube_rejected = 0;               // v10: wedge candidates considered but outside tube radius
    long long wedge_edge_fallback_hits = 0;          // v10: legacy edge-hit diffraction fallback count
    int diffraction_rays_per_event = 4;              // v10: Keller cone samples requested per event
    long long diffraction_events = 0;                // v10: coupled wedge events expanded into Keller samples
    long long generated_diffraction_branches = 0;    // v10: diffraction successor states inserted into wavefront
    long long rejected_keller_diffractions = 0;      // v10: Keller samples rejected by degeneracy/residual guards
    bool path_dedup_enabled = true;                  // v10: signature dedup active during/post tracing
    bool path_similarity_pruning_enabled = true;     // v10: near-equivalent path merge active
    int path_top_n_per_rx = 0;                       // v10: final top-N retained per Rx; <=0 disabled
    long long paths_pruned_by_post_dedup = 0;        // v10: duplicates removed after thread-local recording
    long long paths_pruned_by_similarity = 0;        // v10: near-equivalent paths removed by length/sequence key
    long long paths_pruned_by_top_n = 0;             // v10: paths dropped by final top-N
    long long paths_after_postprocess = 0;           // v10: final recorded path count after pruning
    bool path_residual_filter_enabled = false;       // v10: reject paths above residual thresholds
    long long paths_evaluated_for_residual = 0;      // v10: paths with geometry residual diagnostics computed
    long long paths_pruned_by_residual = 0;          // v10: paths rejected by geometry residual thresholds
    double max_path_geometry_residual = 0.0;         // v10: maximum residual observed after evaluation
    std::string convergence_notes;
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

class SbrEngine {
public:
    // Current v11 geometry pathfinding entry: independent-SBR-style deterministic P2P tracing.
    SbrCoverageResult RunPointToPoint(const SbrContext& context) const;
};

} // namespace rt
