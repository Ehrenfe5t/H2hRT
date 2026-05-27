// 文件目标：
// - 声明模块2面元双侧材质解析接口。
//
// 主要功能：
// - 根据对象命名、对象类别、规则文件和默认介质恢复每个面元的 front/back 材质；
// - 生成对象级 `SceneMaterialBinding` 结果；
// - 为 transmission 主链提供模块2权威双侧材质接口。

#pragma once

#include "MaterialRuleLoader.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 对场景面元执行双侧材质解析。
/// </summary>
/// <param name="ruleSet">已加载的材质规则集合。</param>
/// <param name="scene">待写入双侧材质结果的场景对象。</param>
/// <returns>无返回值。</returns>
void ResolveFaceDualSideMaterial(const SceneMaterialRuleSet& ruleSet, Scene& scene);

} // namespace rt
