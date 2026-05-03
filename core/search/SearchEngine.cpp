// 文件目标：
// - 实现模块4批次5的 SearchEngine 骨架。
//
// 主要功能：
// - 初始化发射状态；
// - 执行最小 DFS 主循环框架；
// - 在批次5中先完成 LOS 路径闭环，为批次6扩展器接入做准备。

#include "SearchEngine.h"

#include "DiffractionExpander.h"
#include "GeometryValidity.h"
#include "ReflectionExpander.h"
#include "TransmissionExpander.h"
#include "StateSignatureBuilder.h"
#include "PathSignatureBuilder.h"

#include <cmath>
#include <set>
#include <sstream>
#include <vector>

namespace rt {

namespace {

Vec3 MakeVec3(double x, double y, double z)
{
    Vec3 value;
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

Vec3 Subtract(const Point3& a, const Point3& b)
{
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 Scale(const Vec3& value, double factor)
{
    return MakeVec3(value.x * factor, value.y * factor, value.z * factor);
}

Point3 Add(const Point3& a, const Vec3& b)
{
    Point3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double Length(const Vec3& value)
{
    return std::sqrt(Dot(value, value));
}

Vec3 Normalize(const Vec3& value)
{
    const double length = Length(value);
    if (length <= 0.0)
    {
        return Vec3{};
    }
    return Scale(value, 1.0 / length);
}

FaceQueryContext BuildFaceQueryContext(const PathState& state, const AppConfig& config)
{
    FaceQueryContext context;
    context.ignored_face_id = state.ignored_face_id;
    context.ignored_object_id = state.ignored_object_id;
    context.ignore_origin_self_hit = true;
    context.origin_ignore_distance = config.numeric_tolerance.self_hit_ignore_distance;
    return context;
}

VisibilityQueryContext BuildVisibilityQueryContext(const PathState& state, const AppConfig& config)
{
    VisibilityQueryContext context;
    context.ignored_face_id = state.last_hit_face_id;
    context.ignored_object_id = state.ignored_object_id;
    context.ignore_origin_attached_face = true;
    context.ignore_target_attached_face = true;
    context.origin_offset_distance = config.numeric_tolerance.visibility_origin_offset;
    context.target_shrink_distance = config.numeric_tolerance.visibility_target_shrink;
    return context;
}

PathState BuildInitialState(const PathSearchContext& context)
{
    PathState state;
    state.current_point = context.tx_point;
    state.current_direction = Normalize(Subtract(context.rx_point, context.tx_point));
    state.current_medium_id = 0;
    state.last_interaction_type = InteractionType::Tx;
    state.accumulated_length = 0.0;
    state.path_depth = 0;
    state.remaining_total_expansions = context.config->path_search.max_path_depth;
    state.remaining_reflections = context.config->path_search.max_reflection_count;
    state.remaining_transmissions = context.config->path_search.max_transmission_count;
    state.remaining_diffractions = context.config->path_search.max_diffraction_count;
    state.remaining_scatterings = context.config->path_search.max_scattering_count;
    state.allow_reflection = context.config->path_search.enable_reflection;
    state.allow_transmission = context.config->path_search.enable_transmission;
    state.allow_diffraction = context.config->path_search.enable_diffraction;
    state.allow_scattering = context.config->path_search.enable_scattering;

    PathNode txNode;
    txNode.interaction_type = InteractionType::Tx;
    txNode.point = context.tx_point;
    txNode.direction = state.current_direction;
    txNode.valid = true;
    state.traversed_nodes.push_back(txNode);
    state.valid = true;
    return state;
}

bool TryBuildLosPath(const PathSearchContext& context, const PathState& state, GeometricPath& path, std::string& traceLine)
{
    const GeometryValidityResult validity = IsValidLosPath(context, state);
    if (!validity.valid)
    {
        traceLine = "LOS invalid: " + validity.detail;
        return false;
    }

    const VisibilityQueryContext visibilityContext = BuildVisibilityQueryContext(state, *context.config);
    if (!context.scene_query->IsVisible(context.tx_point, context.rx_point, visibilityContext))
    {
        traceLine = "LOS blocked by scene occlusion query.";
        return false;
    }

    path.path_id = 0;
    path.total_length = Length(Subtract(context.rx_point, context.tx_point));
    path.is_los = true;

    path.nodes = state.traversed_nodes;
    PathNode rxNode;
    rxNode.interaction_type = InteractionType::Rx;
    rxNode.point = context.rx_point;
    rxNode.direction = state.current_direction;
    rxNode.segment_length_from_previous = path.total_length;
    rxNode.valid = true;
    path.nodes.push_back(rxNode);
    path.valid = true;
    traceLine = "LOS path established from Tx to Rx.";
    return true;
}

void AccumulateFailureReasons(SearchEngineResult& result, const ExpanderResult& expanderResult)
{
    for (GeometryValidityReason reason : expanderResult.failure_reasons)
    {
        ++result.failure_reason_counts[static_cast<int>(reason)];
    }
}

} // namespace

/// <summary>
/// 根据上下文执行批次5范围内的几何搜索。
/// </summary>
/// <param name="context">一次搜索所需的统一上下文。</param>
/// <returns>结构化搜索结果。</returns>
SearchEngineResult SearchEngine::Run(const PathSearchContext& context) const
{
    SearchEngineResult result;
    if (context.config == nullptr || context.scene == nullptr || context.scene_query == nullptr)
    {
        result.trace_lines.push_back("SearchEngine aborted because search context is incomplete.");
        return result;
    }

    std::vector<PathState> stack;
    std::set<std::string> stateSignatures;
    std::set<std::string> pathSignatures;

    PathState initialState = BuildInitialState(context);
    initialState.state_signature = BuildStateSignature(initialState, *context.config);
    stateSignatures.insert(initialState.state_signature);
    stack.push_back(initialState);
    result.generated_state_count = 1;
    result.trace_lines.push_back("Initial PathState constructed.");

    while (!stack.empty())
    {
        PathState currentState = stack.back();
        stack.pop_back();

        const GeometryValidityResult stateExpandable = IsSearchStateExpandable(currentState);
        if (!stateExpandable.valid)
        {
            ++result.failure_reason_counts[static_cast<int>(stateExpandable.reason)];
            result.trace_lines.push_back("State not expandable: " + stateExpandable.detail);
            continue;
        }

        std::ostringstream stateTrace;
        stateTrace << "DFS pop state: depth=" << currentState.path_depth
                   << ", signature=" << currentState.state_signature;
        result.trace_lines.push_back(stateTrace.str());

        if (context.config->path_search.enable_los)
        {
            GeometricPath path;
            std::string losTrace;
            if (TryBuildLosPath(context, currentState, path, losTrace))
            {
                path.path_signature = BuildPathSignature(path, *context.config);
                if (pathSignatures.insert(path.path_signature).second)
                {
                    result.path_set.paths.push_back(path);
                    result.trace_lines.push_back(losTrace);
                }
                else
                {
                    ++result.deduplicated_path_count;
                    result.trace_lines.push_back("LOS path deduplicated by path signature.");
                }
            }
            else
            {
                result.trace_lines.push_back(losTrace);
            }
        }

        if (currentState.remaining_total_expansions <= 0)
        {
            result.trace_lines.push_back("State expansion stopped because remaining_total_expansions <= 0.");
            continue;
        }

        const FaceQueryContext faceContext = BuildFaceQueryContext(currentState, *context.config);
        Ray ray;
        ray.origin = currentState.current_point;
        ray.direction = currentState.current_direction;
        const FaceHit closestHit = context.scene_query->QueryClosestFaceHit(ray, faceContext);

        std::ostringstream hitTrace;
        hitTrace << "Closest face query result: hit=" << (closestHit.hit ? "true" : "false");
        if (closestHit.hit)
        {
            hitTrace << ", face_id=" << closestHit.face_id << ", distance=" << closestHit.distance;
        }
        result.trace_lines.push_back(hitTrace.str());

        const std::vector<WedgeCandidate> wedgeCandidates =
            context.scene_query->QueryCandidateWedges(currentState.current_point, WedgeQueryContext{});
        std::ostringstream wedgeTrace;
        wedgeTrace << "Wedge candidate query result: count=" << wedgeCandidates.size();
        result.trace_lines.push_back(wedgeTrace.str());

        const ExpanderResult reflectionResult = ExpandReflection(context, currentState);
        AccumulateFailureReasons(result, reflectionResult);
        for (const PathState& nextState : reflectionResult.next_states)
        {
            if (stateSignatures.insert(nextState.state_signature).second)
            {
                stack.push_back(nextState);
                ++result.generated_state_count;
            }
            else
            {
                ++result.deduplicated_state_count;
            }
        }

        const ExpanderResult transmissionResult = ExpandTransmission(context, currentState);
        AccumulateFailureReasons(result, transmissionResult);
        for (const PathState& nextState : transmissionResult.next_states)
        {
            if (stateSignatures.insert(nextState.state_signature).second)
            {
                stack.push_back(nextState);
                ++result.generated_state_count;
            }
            else
            {
                ++result.deduplicated_state_count;
            }
        }

        const ExpanderResult diffractionResult = ExpandDiffraction(context, currentState);
        AccumulateFailureReasons(result, diffractionResult);
        for (const PathState& nextState : diffractionResult.next_states)
        {
            if (stateSignatures.insert(nextState.state_signature).second)
            {
                stack.push_back(nextState);
                ++result.generated_state_count;
            }
            else
            {
                ++result.deduplicated_state_count;
            }
        }
    }

    result.succeeded = true;
    return result;
}

} // namespace rt
