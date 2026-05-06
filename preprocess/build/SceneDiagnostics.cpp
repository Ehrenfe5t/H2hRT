// 文件目标：
// - 实现模块2批次3的场景诊断构建器。
//
// 主要功能：
// - 汇总面、边、对象级关键异常；
// - 提供 passed 结论，避免关键问题静默流入后续模块；
// - 支持批次3日志输出和验证闭环。

#include "SceneDiagnostics.h"

#include <map>
#include <sstream>

namespace rt {

/// <summary>
/// 生成场景诊断结果。
/// </summary>
/// <param name="scene">待分析的场景对象。</param>
/// <returns>生成好的场景诊断对象。</returns>
SceneDiagnostics BuildSceneDiagnostics(const Scene& scene)
{
    SceneDiagnostics diagnostics;

    std::map<std::string, int> faceSignatureCount;
    for (const Face& face : scene.faces)
    {
        if (face.degenerate)
        {
            diagnostics.degenerate_faces.push_back(face.face_id);
        }

        if (!face.dual_side_material_resolved)
        {
            diagnostics.faces_missing_dual_side_material.push_back(face.face_id);
        }

        if (face.transmission_enabled && !face.transmission_semantic_complete)
        {
            diagnostics.transmission_faces_missing_semantics.push_back(face.face_id);
        }

        if (face.normal_index < 0)
        {
            diagnostics.flipped_normal_faces.push_back(face.face_id);
        }

        std::ostringstream signature;
        signature << face.vertex_index0 << "," << face.vertex_index1 << "," << face.vertex_index2
                  << "|" << face.object_id;
        const int count = ++faceSignatureCount[signature.str()];
        if (count > 1)
        {
            diagnostics.duplicated_faces.push_back(face.face_id);
        }
    }

    for (const Edge& edge : scene.edges)
    {
        if (edge.is_non_manifold)
        {
            diagnostics.non_manifold_edges.push_back(edge.edge_id);
        }
    }

    for (const SceneMaterialBinding& binding : scene.material_bindings)
    {
        if (binding.rule_name.empty())
        {
            diagnostics.objects_missing_material_mapping.push_back(binding.object_name);
        }

        if (binding.rule_match_mode == "pattern")
        {
            diagnostics.objects_matched_by_pattern.push_back(binding.object_name);
        }

        if (binding.used_default_front_material || binding.used_default_back_material)
        {
            diagnostics.objects_resolved_with_default_materials.push_back(binding.object_name);
        }

        if (binding.recovery_quality_tag == "resolved_partial")
        {
            diagnostics.objects_with_partial_semantic_recovery.push_back(binding.object_name);
        }

        if (binding.recovery_quality_tag == "unresolved")
        {
            diagnostics.unresolved_binding_objects.push_back(binding.object_name);
        }

        if (!binding.transmission_semantic_complete && !binding.back_material_name.empty())
        {
            diagnostics.transmission_objects_missing_semantics.push_back(binding.object_name);
        }
    }

    if (!diagnostics.objects_matched_by_pattern.empty())
    {
        diagnostics.warnings.push_back("Some objects were recovered by pattern rule rather than exact object_name rule.");
    }

    if (!diagnostics.objects_resolved_with_default_materials.empty())
    {
        diagnostics.warnings.push_back("Some objects relied on default front/back material filling during semantic recovery.");
    }

    if (!diagnostics.objects_with_partial_semantic_recovery.empty())
    {
        diagnostics.warnings.push_back("Some objects only reached partial semantic recovery and should be manually reviewed.");
    }

    if (!diagnostics.unresolved_binding_objects.empty())
    {
        diagnostics.warnings.push_back("Some objects remain unresolved after material binding and should be treated as scene-quality blockers.");
    }

    diagnostics.passed = diagnostics.degenerate_faces.empty() &&
                         diagnostics.non_manifold_edges.empty() &&
                         diagnostics.faces_missing_dual_side_material.empty() &&
                         diagnostics.transmission_faces_missing_semantics.empty() &&
                         diagnostics.unresolved_binding_objects.empty() &&
                         diagnostics.objects_with_partial_semantic_recovery.empty();
    return diagnostics;
}

} // namespace rt
