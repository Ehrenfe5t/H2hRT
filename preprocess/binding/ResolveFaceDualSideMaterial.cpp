// 文件目标：
// - 实现模块2面元双侧材质恢复逻辑。
//
// 主要功能：
// - 根据对象级规则批量为对象下所有面元赋予 front/back 材质；
// - 同步生成对象级 `SceneMaterialBinding`；
// - 显式标记哪些面元双侧材质已成功解析。

#include "ResolveFaceDualSideMaterial.h"

namespace rt {

namespace {

/// <summary>
/// 根据对象名查找匹配的材质规则。
/// </summary>
/// <param name="ruleSet">规则集合。</param>
/// <param name="objectName">对象名。</param>
/// <returns>若找到匹配规则则返回其地址，否则返回空指针。</returns>
const SceneMaterialRule* FindRuleForObject(const SceneMaterialRuleSet& ruleSet, const std::string& objectName)
{
    for (const SceneMaterialRule& rule : ruleSet.rules)
    {
        if (rule.object_name == objectName)
        {
            return &rule;
        }
    }

    return nullptr;
}

} // namespace

/// <summary>
/// 对场景面元执行双侧材质解析。
/// </summary>
/// <param name="ruleSet">已加载的材质规则集合。</param>
/// <param name="scene">待写入双侧材质结果的场景对象。</param>
/// <returns>无返回值。</returns>
void ResolveFaceDualSideMaterial(const SceneMaterialRuleSet& ruleSet, Scene& scene)
{
    scene.material_bindings.clear();

    for (const SceneObjectRecord& objectRecord : scene.objects)
    {
        SceneMaterialBinding binding;
        binding.object_id = objectRecord.object_id;
        binding.object_name = objectRecord.object_name;
        binding.face_ids = objectRecord.face_ids;

        const SceneMaterialRule* rule = FindRuleForObject(ruleSet, objectRecord.object_name);
        if (rule == nullptr)
        {
            for (const int faceId : objectRecord.face_ids)
            {
                if (faceId >= 0 && faceId < static_cast<int>(scene.faces.size()))
                {
                    scene.faces[faceId].dual_side_material_resolved = false;
                    scene.faces[faceId].front_material_name.clear();
                    scene.faces[faceId].back_material_name.clear();
                    binding.face_dual_side_resolved_flags.push_back(false);
                }
            }

            scene.material_bindings.push_back(binding);
            continue;
        }

        binding.rule_name = rule->rule_name;
        binding.object_type = rule->object_type;
        binding.surface_material_name = rule->surface_material_name;
        binding.front_material_name = rule->front_material_name.empty() ? ruleSet.default_medium : rule->front_material_name;
        binding.back_material_name = rule->back_material_name.empty() ? rule->surface_material_name : rule->back_material_name;
        binding.normal_rule_tag = rule->normal_rule;

        for (const int faceId : objectRecord.face_ids)
        {
            if (faceId >= 0 && faceId < static_cast<int>(scene.faces.size()))
            {
                Face& face = scene.faces[faceId];
                face.object_type = binding.object_type;
                face.surface_material_name = binding.surface_material_name;
                face.front_material_name = binding.front_material_name;
                face.back_material_name = binding.back_material_name;
                face.normal_rule_tag = binding.normal_rule_tag;
                face.dual_side_material_resolved = true;
                face.reflection_enabled = rule->reflection_enabled;
                face.transmission_enabled = rule->transmission_enabled;
                face.diffraction_candidate_enabled = rule->diffraction_candidate_enabled;
                face.propagation_flags = FacePropagationNone;
                if (face.reflection_enabled)
                {
                    face.propagation_flags |= FacePropagationReflect;
                }
                if (face.transmission_enabled)
                {
                    face.propagation_flags |= FacePropagationTransmit;
                }
                if (face.diffraction_candidate_enabled)
                {
                    face.propagation_flags |= FacePropagationDiffractionBoundary;
                }
                if (face.dual_side_material_resolved)
                {
                    face.propagation_flags |= FacePropagationDualSideResolved;
                }
                binding.face_dual_side_resolved_flags.push_back(true);
            }
        }

        scene.material_bindings.push_back(binding);
    }
}

} // namespace rt
