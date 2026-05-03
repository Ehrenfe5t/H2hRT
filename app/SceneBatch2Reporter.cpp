// 文件目标：
// - 实现批次2场景语义层验证输出函数。
//
// 主要功能：
// - 把 SceneMeta、对象级绑定与面元双侧材质摘要输出到统一日志；
// - 满足主开发文档中批次2的检测方式要求。

#include "SceneBatch2Reporter.h"

#include <sstream>

namespace rt {

/// <summary>
/// 将批次2场景语义层摘要输出到日志系统。
/// </summary>
/// <param name="scene">待输出摘要的场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch2Summary(const Scene& scene, Logger& logger)
{
    std::ostringstream metaStream;
    metaStream << "SceneMeta: objects=" << scene.meta.object_count
               << ", vertices=" << scene.meta.vertex_count
               << ", normals=" << scene.meta.normal_count
               << ", faces=" << scene.meta.face_count;
    logger.Log(LogLevel::Info, "Module2", metaStream.str());

    for (const SceneMaterialBinding& binding : scene.material_bindings)
    {
        std::ostringstream bindingStream;
        bindingStream << "Binding: object_id=" << binding.object_id
                      << ", object_name=" << binding.object_name
                      << ", object_type=" << binding.object_type
                      << ", surface_material=" << binding.surface_material_name
                      << ", front_material=" << binding.front_material_name
                      << ", back_material=" << binding.back_material_name
                      << ", resolved_faces=" << binding.face_dual_side_resolved_flags.size();
        logger.Log(LogLevel::Info, "Module2", bindingStream.str());
    }

    int reportedCount = 0;
    for (const Face& face : scene.faces)
    {
        if (reportedCount >= 12)
        {
            break;
        }

        std::ostringstream faceStream;
        faceStream << "FaceSummary: face_id=" << face.face_id
                   << ", object_name=" << face.object_name
                   << ", front_material=" << face.front_material_name
                   << ", back_material=" << face.back_material_name
                   << ", resolved=" << (face.dual_side_material_resolved ? "true" : "false");
        logger.Log(LogLevel::Info, "Module2", faceStream.str());
        ++reportedCount;
    }
}

} // namespace rt
