// 文件目标：
// - 声明模块2批次3的场景楔边构建器。
//
// 主要功能：
// - 从已构建 Edge 集合中恢复可绕射楔边；
// - 按配置过滤共面边、非流形来源与角度异常边；
// - 为后续加速记录准备稳定楔边真源。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 根据 Edge 集合构建 Scene 楔边集合。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入楔边构建结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneWedges(const AppConfig& config, Scene& scene);

} // namespace rt
