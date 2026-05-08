// 文件目标：
// - 声明模块2场景导入场景基础语义层构建入口。
//
// 主要功能：
// - 组织 OBJ 导入、材质规则加载、对象级材质绑定三个子步骤；
// - 返回可继续进入批次3的 Scene 基础语义层闭环结果；
// - 为 app 层提供单入口调用方式。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/common/error/RtError.h"
#include "../../core/scene/Scene.h"

#include <vector>

namespace rt {

/// <summary>
/// 场景导入场景构建结果。
/// </summary>
struct SceneBatch2BuildResult {
    bool succeeded = false;
    Scene scene;
    std::vector<RtError> errors;
};

/// <summary>
/// 按场景导入要求构建 Scene 基础语义层。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>结构化场景构建结果。</returns>
SceneBatch2BuildResult BuildSceneForBatch2(const AppConfig& config);

} // namespace rt
