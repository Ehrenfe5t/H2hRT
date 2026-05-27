// 文件目标：
// - 声明模块2批次4的场景缓存读写与命中判断入口。
//
// 主要功能：
// - 生成 SceneCacheMeta；
// - 判断缓存是否命中；
// - 读写 SceneCacheContent；
// - 为 app 层和模块2预处理主线提供统一 cache 能力。

#pragma once

#include "SceneCacheContent.h"
#include "../../core/common/config/AppConfig.h"
#include "../../core/common/error/RtError.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 场景缓存加载结果。
/// </summary>
struct SceneCacheLoadResult {
    bool succeeded = false;
    bool cache_hit = false;
    SceneCacheContent content;
    std::vector<RtError> errors;
};

/// <summary>
/// 场景缓存写出结果。
/// </summary>
struct SceneCacheWriteResult {
    bool succeeded = false;
    SceneCacheMeta meta;
    std::vector<RtError> errors;
};

/// <summary>
/// 构建当前场景的缓存元信息。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待摘要的场景对象。</param>
/// <returns>当前场景对应的缓存元信息。</returns>
SceneCacheMeta BuildSceneCacheMeta(const AppConfig& config, const Scene& scene);

/// <summary>
/// 尝试从缓存加载场景内容。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>结构化缓存加载结果。</returns>
SceneCacheLoadResult TryLoadSceneCache(const AppConfig& config);

/// <summary>
/// 将场景内容写入缓存。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待缓存的场景对象。</param>
/// <returns>结构化缓存写出结果。</returns>
SceneCacheWriteResult WriteSceneCache(const AppConfig& config, const Scene& scene);

/// <summary>
/// 获取场景缓存元信息文件路径。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>缓存 meta 文件路径。</returns>
std::string BuildSceneCacheMetaFilePath(const AppConfig& config);

/// <summary>
/// 获取场景缓存内容文件路径。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>缓存 content 文件路径。</returns>
std::string BuildSceneCacheContentFilePath(const AppConfig& config);

} // namespace rt
