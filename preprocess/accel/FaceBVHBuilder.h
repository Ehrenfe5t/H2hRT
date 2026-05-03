// 文件目标：
// - 声明模块2批次3的面元 BVH 构建器。
//
// 主要功能：
// - 基于所有有效面元建立 AABB-BVH；
// - 生成面元查询记录；
// - 统计节点数量、叶节点数量与深度信息。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 构建面元 BVH 及其查询记录。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待读取面元并写回面元加速结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildFaceBVHAcceleration(const AppConfig& config, Scene& scene);

} // namespace rt
