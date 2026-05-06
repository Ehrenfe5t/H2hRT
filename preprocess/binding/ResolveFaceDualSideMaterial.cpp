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

bool WildcardMatch(const std::string& pattern, const std::string& text)
{
    if (pattern.empty())
    {
        return false;
    }

    const std::size_t starPos = pattern.find('*');
    if (starPos == std::string::npos)
    {
        return pattern == text;
    }

    const std::string prefix = pattern.substr(0, starPos);
    const std::string suffix = pattern.substr(starPos + 1U);
    if (text.size() < prefix.size() + suffix.size())
    {
        return false;
    }
    if (!prefix.empty() && text.compare(0, prefix.size(), prefix) != 0)
    {
        return false;
    }
    if (!suffix.empty() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) != 0)
    {
        return false;
    }
    return true;
}

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
        if (!rule.object_name.empty() && rule.object_name == objectName)
        {
            return &rule;
        }
    }

    for (const SceneMaterialRule& rule : ruleSet.rules)
    {
        if (WildcardMatch(rule.object_name_pattern, objectName))
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
            binding.rule_match_mode = "unresolved";
            binding.recovery_quality_tag = "unresolved";
            binding.unresolved_reason = "no_matching_rule";
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
        binding.rule_match_mode = rule->object_name == objectRecord.object_name ? "exact" : "pattern";
        binding.surface_material_name = rule->surface_material_name;
        binding.surface_material_id = rule->surface_material_id;
        binding.front_material_name = rule->front_material_name.empty() ? ruleSet.default_medium : rule->front_material_name;
        binding.front_material_id = rule->front_material_id;
        binding.front_medium_id = rule->front_medium_id;
        binding.back_material_name = rule->back_material_name.empty() ? rule->surface_material_name : rule->back_material_name;
        binding.back_material_id = rule->back_material_id;
        binding.back_medium_id = rule->back_medium_id;
        binding.normal_rule_tag = rule->normal_rule;
        binding.used_default_front_material = rule->front_material_name.empty();
        binding.used_default_back_material = rule->back_material_name.empty();
        binding.transmission_semantic_complete = !binding.front_material_name.empty() &&
                                                 !binding.back_material_name.empty() &&
                                                 binding.front_medium_id >= 0 &&
                                                 binding.back_medium_id >= 0;
        binding.recovery_quality_tag = binding.transmission_semantic_complete
            ? ((binding.used_default_front_material || binding.used_default_back_material) ? "resolved_with_defaults" : "resolved_full")
            : "resolved_partial";

        for (const int faceId : objectRecord.face_ids)
        {
            if (faceId >= 0 && faceId < static_cast<int>(scene.faces.size()))
            {
                Face& face = scene.faces[faceId];
                face.object_type = binding.object_type;
                face.surface_material_name = binding.surface_material_name;
                face.surface_material_id = binding.surface_material_id;
                face.front_material_name = binding.front_material_name;
                face.front_material_id = binding.front_material_id;
                face.front_medium_id = binding.front_medium_id;
                face.back_material_name = binding.back_material_name;
                face.back_material_id = binding.back_material_id;
                face.back_medium_id = binding.back_medium_id;
                face.normal_rule_tag = binding.normal_rule_tag;
                face.dual_side_material_resolved = true;
                face.reflection_enabled = rule->reflection_enabled;
                face.transmission_enabled = rule->transmission_enabled;
                face.diffraction_candidate_enabled = rule->diffraction_candidate_enabled;
                face.transmission_semantic_complete = !face.front_material_name.empty() &&
                                                      !face.back_material_name.empty() &&
                                                      face.front_medium_id >= 0 &&
                                                      face.back_medium_id >= 0;
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
