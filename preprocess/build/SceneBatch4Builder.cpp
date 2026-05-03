// 文件目标：
// - 实现模块2批次4查询门面与缓存闭环入口。
//
// 主要功能：
// - 优先尝试 SceneCache 命中；
// - 未命中时对批次3结果接线 SceneQuery 并写回缓存；
// - 对 cache 读写失败保持显式错误，不允许关键静默回退。

#include "SceneBatch4Builder.h"

#include "../../core/query/SceneQuery.h"
#include "../cache/SceneCache.h"

#include <memory>

namespace rt {

namespace {

/// <summary>
/// 为场景重新绑定当前生命周期有效的查询门面。
/// </summary>
/// <param name="scene">待接线查询门面的场景对象。</param>
/// <param name="config">统一应用配置对象。</param>
/// <returns>无返回值。</returns>
void AttachSceneQuery(Scene& scene, const AppConfig& config)
{
    scene.query.reset();
    scene.query = std::make_shared<SceneQuery>(scene, config);
}

} // namespace

/// <summary>
/// 按批次4要求构建 SceneQuery 与 SceneCache 闭环。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="batch3Scene">已完成批次3的场景对象。</param>
/// <returns>结构化批次4构建结果。</returns>
SceneBatch4BuildResult BuildSceneForBatch4(const AppConfig& config, const Scene& batch3Scene)
{
    SceneBatch4BuildResult result;

    if (config.scene_preprocess.enable_scene_cache)
    {
        const SceneCacheLoadResult loadResult = TryLoadSceneCache(config);
        for (const RtError& error : loadResult.errors)
        {
            result.errors.push_back(error);
        }

        if (!loadResult.errors.empty())
        {
            return result;
        }

        if (loadResult.cache_hit && loadResult.succeeded)
        {
            result.succeeded = true;
            result.cache_hit = true;
            result.scene = loadResult.content.scene;
            AttachSceneQuery(result.scene, config);
            result.cache_meta = loadResult.content.meta;
            return result;
        }
    }

    result.scene = batch3Scene;
    AttachSceneQuery(result.scene, config);
    result.cache_meta = BuildSceneCacheMeta(config, result.scene);

    if (config.scene_preprocess.enable_scene_cache)
    {
        const SceneCacheWriteResult writeResult = WriteSceneCache(config, result.scene);
        for (const RtError& error : writeResult.errors)
        {
            result.errors.push_back(error);
        }
        if (!writeResult.errors.empty())
        {
            return result;
        }
        result.cache_meta = writeResult.meta;
    }

    result.succeeded = true;
    return result;
}

} // namespace rt
