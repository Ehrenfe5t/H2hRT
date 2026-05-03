// 文件目标：
// - 实现模块2对象级材质绑定主过程。
//
// 主要功能：
// - 调用双侧材质恢复逻辑；
// - 保持模块2批次2对象级绑定入口集中明确。

#include "SceneMaterialBinding.h"

#include "../binding/ResolveFaceDualSideMaterial.h"

namespace rt {

/// <summary>
/// 根据规则集合为场景执行对象级材质绑定。
/// </summary>
/// <param name="ruleSet">已加载的材质规则集合。</param>
/// <param name="scene">待绑定的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneMaterialBinding(const SceneMaterialRuleSet& ruleSet, Scene& scene)
{
    ResolveFaceDualSideMaterial(ruleSet, scene);
}

} // namespace rt
