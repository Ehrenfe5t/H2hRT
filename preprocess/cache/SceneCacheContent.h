// 文件目标：
// - 定义模块2批次4的场景缓存内容结构。
//
// 主要功能：
// - 显式表达缓存中的 Scene 核心内容；
// - 为 cache 序列化与恢复保持统一结构边界；
// - 避免只缓存 BVH 而丢失语义、拓扑和诊断层。

#pragma once

#include "SceneCacheMeta.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 场景缓存内容结构。
/// </summary>
struct SceneCacheContent {
    SceneCacheMeta meta;
    Scene scene;
};

} // namespace rt
