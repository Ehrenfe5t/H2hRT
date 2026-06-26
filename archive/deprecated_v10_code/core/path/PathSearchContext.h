// 文件目标：
// - 定义模块4几何寻径阶段使用的统一搜索上下文对象。
//
// 主要功能：
// - 汇总配置、场景、查询门面、材质数据库与收发点位置；
// - 作为路径搜索状态机的全局只读输入容器；
// - 避免各搜索子过程重复传递参数列表。

#pragma once

#include "PathState.h"
#include "../common/config/AppConfig.h"
#include "../common/material/MaterialDatabase.h"
#include "../query/SceneQuery.h"
#include "../scene/Scene.h"

namespace rt {

/// <summary>
/// 路径搜索上下文，聚合搜索过程中所需的只读全局资源。
/// </summary>
struct PathSearchContext {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const SceneQuery* scene_query = nullptr;
    const MaterialDatabase* material_db = nullptr;
    Point3 tx_point;   // 发射天线位置
    Point3 rx_point;   // 接收天线位置
};

} // namespace rt
