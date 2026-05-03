// 文件目标：
// - 声明模块2对象级材质绑定过程接口。
//
// 主要功能：
// - 把规则加载和双侧材质推导封装为单个场景绑定过程；
// - 为 app 层提供模块2批次2的一步式绑定调用入口。

#pragma once

#include "../binding/MaterialRuleLoader.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 根据规则集合为场景执行对象级材质绑定。
/// </summary>
/// <param name="ruleSet">已加载的材质规则集合。</param>
/// <param name="scene">待绑定的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneMaterialBinding(const SceneMaterialRuleSet& ruleSet, Scene& scene);

} // namespace rt
