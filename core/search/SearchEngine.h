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

#include <vector>

namespace rt {

/// <summary>
/// 模块4搜索执行结果。
/// </summary>
struct SearchEngineResult {
    bool succeeded = false;
    GeometricPathSet path_set;
    int generated_state_count = 0;
    int deduplicated_state_count = 0;
    int deduplicated_path_count = 0;
    std::map<int, int> failure_reason_counts;
    std::vector<std::string> trace_lines;
};

/// <summary>
/// 模块4几何寻径引擎骨架。
/// </summary>
class SearchEngine {
public:
    /// <summary>
    /// 根据上下文执行批次5范围内的几何搜索。
    /// </summary>
    /// <param name="context">一次搜索所需的统一上下文。</param>
    /// <returns>结构化搜索结果。</returns>
    SearchEngineResult Run(const PathSearchContext& context) const;
};

} // namespace rt
