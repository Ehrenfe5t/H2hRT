// 文件目标：
// - 实现模块2批次2场景基础语义层构建入口。
//
// 主要功能：
// - 调用 OBJ 导入器；
// - 调用材质规则加载器；
// - 调用对象级材质绑定流程；
// - 形成批次2的最小可运行 Scene 语义层闭环。

#include "SceneBatch2Builder.h"

#include "SceneMaterialBinding.h"
#include "../binding/MaterialRuleLoader.h"
#include "../import/OBJImporter.h"

namespace rt {

/// <summary>
/// 按批次2要求构建 Scene 基础语义层。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>结构化场景构建结果。</returns>
SceneBatch2BuildResult BuildSceneForBatch2(const AppConfig& config)
{
    SceneBatch2BuildResult result;

    const OBJImportResult importResult = ImportSceneFromOBJ(config.scene_import.source_file,
                                                            config.scene_import.coordinate_transform);
    for (const RtError& error : importResult.errors)
    {
        result.errors.push_back(error);
    }

    if (!importResult.succeeded)
    {
        return result;
    }

    result.scene = importResult.scene;

    // C4-B: Coordinate transform (Blender Z-up -> algorithm Y-up)
    if (config.scene_import.coordinate_transform == "blender_z_up_to_y_up") {
        for (auto& v : result.scene.vertices) { double t = v.y; v.y = v.z; v.z = t; }
        for (auto& n : result.scene.normals) { double t = n.y; n.y = n.z; n.z = t; }
    }

    const MaterialRuleLoadResult ruleLoadResult = LoadSceneMaterialRules(config.scene_import.scene_material_map_file);
    for (const RtError& error : ruleLoadResult.errors)
    {
        result.errors.push_back(error);
    }

    if (!ruleLoadResult.succeeded)
    {
        return result;
    }

    BuildSceneMaterialBinding(ruleLoadResult.rule_set, result.scene);
    result.succeeded = true;
    return result;
}

} // namespace rt
