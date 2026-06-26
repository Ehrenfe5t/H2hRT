// v11: DEPRECATED — v8 Phase 4 路径复用引擎, 不参与 v11 P2P 主链.
// 保留用于历史对照.
// Quitin et al., "A Path Reuse Framework for Deterministic Channel Modeling", IEEE TAP 2020
#pragma once

#include "../common/config/AppConfig.h"
#include "../path/GeometricPath.h"
#include "../query/SceneQuery.h"
#include <vector>

namespace rt {

struct PathReuseConfig {
    double max_reuse_distance_m = 2.0;
    bool verify_last_hop = true;
    int max_paths_per_rx = 256;
};

struct PathReuseResult {
    std::vector<GeometricPath> paths;
    int reused_count = 0;
    int verified_count = 0;
    int rejected_visibility = 0;
    int rejected_distance = 0;
};

class PathReuseEngine {
public:
    static PathReuseResult ReusePaths(
        const Point3& refRx,
        const std::vector<GeometricPath>& refPaths,
        const Point3& targetRx,
        const SceneQuery& query,
        const AppConfig& appConfig,
        const PathReuseConfig& config = {});
};

} // namespace rt
