#pragma once

#include "../common/config/AppConfig.h"
#include "../common/material/MaterialDatabase.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"
#include "../query/SceneQuery.h"
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
    int total_rays = 0;
    int active_rx_count = 0;
    std::vector<RxCoverageRecord> rx_records;
    std::vector<std::string> trace_lines;
};

struct SbrContext {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const SceneQuery* scene_query = nullptr;
    const MaterialDatabase* material_db = nullptr;
    Point3 tx_point;
    std::vector<Point3> rx_grid;
    bool store_paths = false;
    double tx_power_w = 1.0;
};

class SbrEngine {
public:
    SbrCoverageResult Run(const SbrContext& context) const;
};

} // namespace rt
