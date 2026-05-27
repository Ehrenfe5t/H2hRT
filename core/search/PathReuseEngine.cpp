// v8 Phase 4: PathReuseEngine 实现
#include "PathReuseEngine.h"
#include "PathSignatureBuilder.h"
#include "../common/config/AppConfig.h"
#include "../common/math/Vec3.h"

#include <unordered_set>

namespace rt {

PathReuseResult PathReuseEngine::ReusePaths(
    const Point3& refRx,
    const std::vector<GeometricPath>& refPaths,
    const Point3& targetRx,
    const SceneQuery& query,
    const AppConfig& appConfig,
    const PathReuseConfig& config)
{
    PathReuseResult result;

    double dist = Length(Subtract(targetRx, refRx));
    if (dist > config.max_reuse_distance_m) {
        result.rejected_distance = static_cast<int>(refPaths.size());
        return result;
    }

    std::unordered_set<uint64_t> seenSignatures;

    for (const GeometricPath& refPath : refPaths) {
        if (!refPath.valid || refPath.nodes.size() < 2) continue;

        const PathNode& lastInteraction = refPath.nodes[refPath.nodes.size() - 2];
        const Point3& lastPoint = lastInteraction.point;

        if (config.verify_last_hop) {
            VisibilityQueryContext vc;
            vc.ignored_face_id = lastInteraction.face_id;
            vc.ignored_object_id = lastInteraction.object_id;
            if (!query.IsVisible(lastPoint, targetRx, vc)) {
                result.rejected_visibility++;
                continue;
            }
        }

        GeometricPath newPath = refPath;
        newPath.nodes.pop_back();

        double lastHopLen = Length(Subtract(targetRx, lastPoint));
        newPath.total_length = refPath.total_length
            - Length(Subtract(refRx, lastPoint))
            + lastHopLen;

        PathNode newRxNode;
        newRxNode.interaction_type = InteractionType::Rx;
        newRxNode.point = targetRx;
        newRxNode.direction = lastInteraction.direction;
        newRxNode.segment_length_from_previous = lastHopLen;
        newRxNode.valid = true;
        newPath.nodes.push_back(newRxNode);

        newPath.path_signature = BuildPathSignature(newPath, appConfig);
        if (seenSignatures.insert(newPath.path_signature).second) {
            result.paths.push_back(newPath);
            result.reused_count++;
            if (static_cast<int>(result.paths.size()) >= config.max_paths_per_rx) break;
        }
    }

    return result;
}

} // namespace rt
