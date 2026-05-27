// 文件目标：
// - 声明模块2拓扑构建的拓扑、诊断与加速结构构建入口。
//
// 主要功能：
// - 在批次2基础语义层之上继续构建 Edge/Wedge/Diagnostics/Acceleration；
// - 聚合各子构建器错误并返回结构化结果；
// - 为 app 层提供拓扑构建单入口调用方式。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/common/error/RtError.h"
#include "../../core/scene/Scene.h"

#include <vector>

namespace rt {

/// <summary>
/// 拓扑构建场景构建结果。
/// </summary>
struct SceneBatch3BuildResult {
    bool succeeded = false;
    Scene scene;
    std::vector<RtError> errors;
};

/// <summary>
/// 按拓扑构建要求构建 Scene 拓扑、诊断与加速结构层。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="batch2Scene">已完成批次2语义恢复的场景对象。</param>
/// <returns>结构化拓扑构建构建结果。</returns>
SceneBatch3BuildResult BuildSceneForBatch3(const AppConfig& config, const Scene& batch2Scene);

} // namespace rt
