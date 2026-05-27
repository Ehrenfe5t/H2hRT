// 文件目标：
// - 声明模块4批次5的 SearchEngine 骨架。
//
// 主要功能：
// - 生成初始 PathState；
// - 执行第一版 DFS 主循环骨架；
// - 在批次5范围内完成最小 LOS 闭环与状态/路径级去重框架。

#pragma once

#include "../path/GeometricPath.h"
#include "../path/PathSearchContext.h"

#include <map>
#include <string>

#include <vector>

namespace rt {

/// <summary>
/// 模块4搜索执行结果。
/// </summary>
struct SearchEngineResult {
    bool succeeded = false;
    GeometricPathSet path_set;
    std::string source_tag = "search_engine_real_output";
    bool uses_real_scene_query = false;
    int control_rule_rejected_state_count = 0;
    int invalid_sequence_rejected_count = 0;
    int mixed_path_blocked_count = 0;
    int mixed_path_generated_count = 0;
    int candidate_state_count = 0;
    int accepted_state_count = 0;
    int truncated_candidate_count = 0;
    int generated_state_count = 0;
    int deduplicated_state_count = 0;
    int deduplicated_path_count = 0;
    std::map<int, int> failure_reason_counts;
    std::vector<std::string> trace_lines;
};

/// <summary>
/// v8: 约束搜索配置 — 将搜索空间限制在指定面元/楔边集内, 大幅降低组合爆炸
/// </summary>
struct ConstrainedSearchConfig {
    const std::vector<int>* candidate_faces = nullptr;   // 面元约束集 (nullptr=全场景)
    const std::vector<int>* candidate_wedges = nullptr;   // 楔边约束集 (nullptr=全场景)
    int max_forward_depth = 5;                            // 双向搜索预留
    bool enable_bidirectional = false;                    // 双向搜索预留 (Phase 3b)
};

/// <summary>
/// 模块4几何寻径引擎骨架。
/// </summary>
class SearchEngine {
public:
    /// <summary>
    /// 根据上下文执行批次5范围内的几何搜索。
    /// </summary>
    SearchEngineResult Run(const PathSearchContext& context) const;

    /// <summary>v8: 约束搜索 — 候选面元/楔边受限</summary>
    SearchEngineResult Run(const PathSearchContext& context,
                           const ConstrainedSearchConfig& constraints) const;
};

} // namespace rt
