// 文件目标：
// - 声明模块2批次3的场景边构建器。
//
// 主要功能：
// - 从三角面恢复唯一拓扑边；
// - 回填面到边的邻接关系；
// - 识别边界边、非流形边与共面边。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 根据 Scene 面元集合构建唯一拓扑边并回填邻接。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入边构建结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneEdges(const AppConfig& config, Scene& scene);

} // namespace rt
