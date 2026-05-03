// 文件目标：
// - 声明模块2批次4查询门面与缓存闭环构建入口。
//
// 主要功能：
// - 组织 SceneQuery 接线、SceneCache 命中与回写；
// - 返回批次4要求的结构化结果与 cache 状态；
// - 为 app 层提供批次4单入口调用方式。

#pragma once

#include "../../core/common/config/AppConfig.h"
#include "../../core/common/error/RtError.h"
#include "../../core/scene/Scene.h"
#include "../cache/SceneCacheMeta.h"

#include <vector>

namespace rt {

/// <summary>
/// 批次4场景构建结果。
/// </summary>
struct SceneBatch4BuildResult {
    bool succeeded = false;
    bool cache_hit = false;
    Scene scene;
    SceneCacheMeta cache_meta;
    std::vector<RtError> errors;
};

/// <summary>
/// 按批次4要求构建 SceneQuery 与 SceneCache 闭环。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="batch3Scene">已完成批次3的场景对象。</param>
/// <returns>结构化批次4构建结果。</returns>
SceneBatch4BuildResult BuildSceneForBatch4(const AppConfig& config, const Scene& batch3Scene);

} // namespace rt
