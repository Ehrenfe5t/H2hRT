// 文件目标：
// - 实现模块2批次3的统一构建入口。
//
// 主要功能：
// - 在批次2结果之上串联 Edge/Wedge/Diagnostics/Acceleration；
// - 将关键失败显式转为 RtError，禁止静默继续；
// - 为 app 层提供批次3闭环的统一返回对象。

#include "SceneBatch3Builder.h"

#include "EdgeBuilder.h"
#include "SceneDiagnostics.h"
#include "WedgeBuilder.h"
#include "../../core/query/SceneQuery.h"
#include "../accel/SceneAcceleration.h"

#include <memory>

namespace rt {

/// <summary>
/// 按批次3要求构建 Scene 拓扑、诊断与加速结构层。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="batch2Scene">已完成批次2语义恢复的场景对象。</param>
/// <returns>结构化批次3构建结果。</returns>
SceneBatch3BuildResult BuildSceneForBatch3(const AppConfig& config, const Scene& batch2Scene)
{
    SceneBatch3BuildResult result;
    result.scene = batch2Scene;

    BuildSceneEdges(config, result.scene);
    BuildSceneWedges(config, result.scene);
    result.scene.diagnostics = BuildSceneDiagnostics(result.scene);
    BuildSceneAcceleration(config, result.scene);
    result.scene.query = std::make_shared<SceneQuery>(result.scene, config);

    if (!result.scene.diagnostics.faces_missing_dual_side_material.empty())
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::ValidationFailed,
            "Module2",
            "Scene diagnostics detected faces missing dual-side materials.",
            "SceneDiagnostics.faces_missing_dual_side_material",
            "Fix scene_material_map.json or object semantic recovery before using topology results.",
            true));
    }

    if (!result.scene.acceleration.diagnostics.build_succeeded)
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::ValidationFailed,
            "Module2",
            "Scene acceleration build did not pass diagnostics.",
            "SceneAccelerationDiagnostics",
            "Inspect BVH validity, brute-force validation result and wedge acceleration warnings/errors.",
            true));
    }

    result.succeeded = result.errors.empty();
    return result;
}

} // namespace rt
