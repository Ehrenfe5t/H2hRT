// 文件目标：
// - 定义模块4批次5的路径搜索上下文对象。
//
// 主要功能：
// - 汇总 Scene、SceneQuery、Tx/Rx 与 Search 配置引用；
// - 作为 SearchEngine 执行一次搜索的统一输入容器；
// - 避免在模块4骨架阶段散落大量全局参数传递。

#pragma once

#include "PathState.h"
#include "../common/config/AppConfig.h"
#include "../query/SceneQuery.h"
#include "../scene/Scene.h"

namespace rt {

/// <summary>
/// 模块4路径搜索上下文对象。
/// </summary>
struct PathSearchContext {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const SceneQuery* scene_query = nullptr;
    Point3 tx_point;
    Point3 rx_point;
};

} // namespace rt
