// 文件目标：
// - 声明模块2批次3的楔边轻量加速记录构建器。
//
// 主要功能：
// - 从 Wedge 真源生成紧凑查询记录；
// - 统计可绕射楔边数量；
// - 为后续 SceneQuery 的候选粗筛保留接口基础。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 构建楔边轻量查询记录集合。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待读取楔边并写回楔边加速结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildWedgeAcceleration(const AppConfig& config, Scene& scene);

} // namespace rt
