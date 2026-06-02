// v10 Iter7: 四阶段混合寻径引擎 V2 — 主入口
// 集成: BidirectionalPVS + InteractionStateMachine + HybridPathSolver + PathValidator

#pragma once

#include "../common/config/AppConfig.h"
#include "SearchEngine.h"  // reuses SearchEngineResult

namespace rt {

struct PathSearchContext;

class PathSearchEngineV2 {
public:
    PathSearchEngineV2() = default;

    /// 执行四阶段寻径
    SearchEngineResult Run(const PathSearchContext& context) const;

    /// 是否启用 V2 (从 AppConfig 读取)
    static bool IsEnabled(const AppConfig& config) {
        return config.path_search.enable_v10_precise_engine;
    }
};

} // namespace rt
