// 文件目标：
// - 实现批次4查询门面与缓存闭环验证输出函数。
//
// 主要功能：
// - 输出 cache 命中/失效状态；
// - 输出 preprocess mode 与 cache meta 摘要；
// - 输出基础查询自检结果，验证 SceneQuery 已可独立使用。

#include "SceneBatch4Reporter.h"

#include "../../core/query/SceneQuery.h"

#include <sstream>

namespace rt {

namespace {

bool RunBatch4QuerySelfCheck(const Scene& scene, Logger& logger)
{
    if (scene.query == nullptr || scene.faces.empty())
    {
        logger.Log(LogLevel::Error, "Module2", "Batch4QuerySelfCheck: SceneQuery unavailable or scene has no faces.");
        return false;
    }

    const Face& face = scene.faces.front();
    if (face.vertex_index0 < 0 || face.vertex_index1 < 0 || face.vertex_index2 < 0)
    {
        logger.Log(LogLevel::Error, "Module2", "Batch4QuerySelfCheck: first face vertex indices are invalid.");
        return false;
    }

    const Point3& p0 = scene.vertices[face.vertex_index0];
    const Point3& p1 = scene.vertices[face.vertex_index1];
    const Point3& p2 = scene.vertices[face.vertex_index2];
    Point3 centroid;
    centroid.x = (p0.x + p1.x + p2.x) / 3.0;
    centroid.y = (p0.y + p1.y + p2.y) / 3.0;
    centroid.z = (p0.z + p1.z + p2.z) / 3.0;

    Ray ray;
    ray.origin = centroid;
    ray.origin.x += face.normal.x * 10.0;
    ray.origin.y += face.normal.y * 10.0;
    ray.origin.z += face.normal.z * 10.0;
    ray.direction.x = -face.normal.x;
    ray.direction.y = -face.normal.y;
    ray.direction.z = -face.normal.z;

    FaceQueryContext context;
    context.origin_ignore_distance = 0.0;

    const FaceHit closestHit = scene.query->QueryClosestFaceHit(ray, context);
    const std::vector<FaceHit> allHits = scene.query->QueryAllFaceHits(ray, context);
    const std::vector<FaceHit> shortRangeHits = scene.query->QueryFaceHitsInRange(ray, 0.0, 5.0, context);

    VisibilityQueryContext visibilityContext;
    visibilityContext.ignored_face_id = face.face_id;
    visibilityContext.ignore_target_attached_face = false;
    const bool visible = scene.query->IsVisible(ray.origin, centroid, visibilityContext);
    const std::vector<WedgeCandidate> wedges = scene.query->QueryCandidateWedges(centroid, WedgeQueryContext{});

    std::ostringstream trace;
    trace << "Batch4QuerySelfCheck: closest_hit=" << (closestHit.hit ? "true" : "false")
          << ", all_hits=" << allHits.size()
          << ", range_hits=" << shortRangeHits.size()
          << ", visible=" << (visible ? "true" : "false")
          << ", wedge_candidates=" << wedges.size();
    logger.Log(LogLevel::Info, "Module2", trace.str());

    return closestHit.hit && !allHits.empty();
}

} // namespace

/// <summary>
/// 将批次4查询门面与缓存摘要输出到日志系统。
/// </summary>
/// <param name="result">批次4构建结果。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch4Summary(const SceneBatch4BuildResult& result, Logger& logger)
{
    std::ostringstream cacheStream;
    cacheStream << "SceneCache: cache_hit=" << (result.cache_hit ? "true" : "false")
                << ", cache_format_version=" << result.cache_meta.cache_format_version
                << ", preprocess_mode_debug=" << (result.cache_meta.contains_debug_auxiliary_data ? "true" : "false")
                << ", replay_ready=" << (result.cache_meta.replay_support_ready ? "true" : "false")
                << ", face_count=" << result.cache_meta.face_count
                << ", edge_count=" << result.cache_meta.edge_count
                << ", wedge_count=" << result.cache_meta.wedge_count
                << ", status_reason=" << result.cache_meta.cache_status_reason;
    logger.Log(LogLevel::Info, "Module2", cacheStream.str());

    const bool selfCheckPassed = RunBatch4QuerySelfCheck(result.scene, logger);
    logger.Log(
        selfCheckPassed ? LogLevel::Info : LogLevel::Error,
        "Module2",
        std::string("Batch4 query self-check ") + (selfCheckPassed ? "passed." : "failed."));
}

} // namespace rt
