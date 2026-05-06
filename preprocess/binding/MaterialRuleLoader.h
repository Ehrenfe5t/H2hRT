// 文件目标：
// - 声明模块2材质映射规则加载接口。
//
// 主要功能：
// - 从 `scene_material_map.json` 读取对象级语义与双侧材质推导规则；
// - 为 `ResolveFaceDualSideMaterial` 提供稳定输入；
// - 在规则缺失或解析失败时返回显式错误结果。

#pragma once

#include "../../core/common/error/RtError.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 单条对象级材质规则。
/// </summary>
struct SceneMaterialRule {
    std::string rule_name;
    std::string object_name;
    std::string object_name_pattern;
    std::string object_type;
    std::string surface_material_name;
    std::string front_material_name;
    std::string back_material_name;
    std::string normal_rule;
    int surface_material_id = -1;
    int front_material_id = -1;
    int back_material_id = -1;
    int front_medium_id = -1;
    int back_medium_id = -1;
    bool reflection_enabled = true;
    bool transmission_enabled = false;
    bool diffraction_candidate_enabled = false;
};

/// <summary>
/// 材质规则集合。
/// </summary>
struct SceneMaterialRuleSet {
    std::string default_medium = "Air";
    int default_medium_id = -1;
    std::vector<SceneMaterialRule> rules;
};

/// <summary>
/// 材质规则加载结果。
/// </summary>
struct MaterialRuleLoadResult {
    bool succeeded = false;
    SceneMaterialRuleSet rule_set;
    std::vector<RtError> errors;
};

/// <summary>
/// 从 JSON 文件加载场景材质映射规则。
/// </summary>
/// <param name="filePath">规则文件路径。</param>
/// <returns>结构化规则加载结果。</returns>
MaterialRuleLoadResult LoadSceneMaterialRules(const std::string& filePath);

} // namespace rt
