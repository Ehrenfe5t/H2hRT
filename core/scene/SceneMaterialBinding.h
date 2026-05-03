// 文件目标：
// - 定义模块2对象级材质绑定与双侧材质解析结果结构。
//
// 主要功能：
// - 记录对象命名、对象类型、主材质、法向规则与规则来源；
// - 保存对象下各面元双侧材质是否成功解析；
// - 为批次2验证输出对象级绑定记录提供统一结构。

#pragma once

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 对象级场景材质绑定结果。
/// </summary>
struct SceneMaterialBinding {
    int object_id = -1;
    std::string object_name;
    std::string object_type;
    std::string surface_material_name;
    std::string front_material_name;
    std::string back_material_name;
    std::string normal_rule_tag;
    std::string rule_name;
    std::vector<int> face_ids;
    std::vector<bool> face_dual_side_resolved_flags;
};

} // namespace rt
