// 文件目标：
// - 声明模块2批次3统一场景加速结构构建入口。
//
// 主要功能：
// - 串联 Face BVH 与 WedgeAcceleration 两类加速构建；
// - 组织构建时间、统计信息与 brute-force 对照结果；
// - 为批次3形成统一 SceneAcceleration 输出。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 构建场景统一加速结构结果。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入加速结构结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneAcceleration(const AppConfig& config, Scene& scene);

} // namespace rt
