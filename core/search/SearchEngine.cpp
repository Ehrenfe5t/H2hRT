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
#include <algorithm>
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

int GetMechanismPriority(InteractionType interactionType)
{
    switch (interactionType)
    {
    case InteractionType::Reflection:
        return 0;
    case InteractionType::Transmission:
        return 1;
    case InteractionType::Diffraction:
        return 2;
    default:
        return 99;
    }
}

double ComputeStateScore(const PathState& state)
{
    double score = 0.0;
    score += static_cast<double>(state.path_depth) * 100.0;
    score += static_cast<double>(state.mechanism_switch_count) * 10.0;
    score += state.accumulated_length * 0.01;
    score += static_cast<double>(GetMechanismPriority(state.last_interaction_type));
    return score;
}

void SortAndTruncateCandidates(std::vector<PathState>& candidates, int keepLimit, SearchEngineResult& result)
{
    std::sort(candidates.begin(), candidates.end(),
        [](const PathState& lhs, const PathState& rhs)
        {
            return ComputeStateScore(lhs) < ComputeStateScore(rhs);
        });

    if (keepLimit > 0 && static_cast<int>(candidates.size()) > keepLimit)
    {
        result.truncated_candidate_count += static_cast<int>(candidates.size()) - keepLimit;
        candidates.resize(static_cast<std::size_t>(keepLimit));
    }
}

void AppendCandidates(
    std::vector<PathState>& acceptedCandidates,
    const ExpanderResult& expanderResult,
    SearchEngineResult& result,
    int perExpanderKeepLimit)
{
    std::vector<PathState> localCandidates = expanderResult.next_states;
    result.candidate_state_count += static_cast<int>(localCandidates.size());
    SortAndTruncateCandidates(localCandidates, perExpanderKeepLimit, result);
    result.accepted_state_count += static_cast<int>(localCandidates.size());
    acceptedCandidates.insert(acceptedCandidates.end(), localCandidates.begin(), localCandidates.end());
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
    state.interaction_count = 0;
    state.remaining_total_expansions = context.config->path_search.max_path_depth;
    state.remaining_reflections = context.config->path_search.max_reflection_count;
    state.remaining_transmissions = context.config->path_search.max_transmission_count;
    state.remaining_diffractions = context.config->path_search.max_diffraction_count;
    state.remaining_scatterings = context.config->path_search.max_scattering_count;
    state.allow_reflection = context.config->path_search.enable_reflection;
    state.allow_transmission = context.config->path_search.enable_transmission;
    state.allow_diffraction = context.config->path_search.enable_diffraction;
    state.allow_scattering = context.config->path_search.enable_scattering;
    state.mixed_path_enabled = false;
    state.mechanism_switch_count = 0;
    state.consecutive_same_interaction_count = 0;

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
    if (!context.scene_query->IsVisible(state.current_point, context.rx_point, visibilityContext))
    {
        traceLine = "LOS / final-leg visibility blocked by scene occlusion query.";
        return false;
    }

    const double finalLegLength = Length(Subtract(context.rx_point, state.current_point));
    path.path_id = 0;
    path.total_length = state.accumulated_length + finalLegLength;
    path.is_los = (state.path_depth == 0);
    path.contains_transmission = state.has_transmission;

    path.nodes = state.traversed_nodes;
    PathNode rxNode;
    rxNode.interaction_type = InteractionType::Rx;
    rxNode.point = context.rx_point;
    rxNode.direction = state.current_direction;
    rxNode.segment_length_from_previous = finalLegLength;
    rxNode.valid = true;
    path.nodes.push_back(rxNode);
    path.valid = true;
    if (state.path_depth == 0)
    {
        traceLine = "LOS path established from Tx to Rx.";
    }
    else
    {
        traceLine = "Geometric path established from current search state to Rx final leg.";
    }
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
    result.uses_real_scene_query = true;
    result.trace_lines.push_back("Initial PathState constructed.");

    while (!stack.empty())
    {
        PathState currentState = stack.back();
        stack.pop_back();

        const GeometryValidityResult stateExpandable = IsSearchStateExpandable(currentState);
        if (!stateExpandable.valid)
        {
            ++result.failure_reason_counts[static_cast<int>(stateExpandable.reason)];
            if (stateExpandable.reason == GeometryValidityReason::CandidateRejectedByControl ||
                stateExpandable.reason == GeometryValidityReason::DuplicateInteractionLoop)
            {
                ++result.control_rule_rejected_state_count;
            }
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

        const int perExpanderKeepLimit = std::max(1, std::min(4, context.config->path_search.max_candidate_face_hits));
        const int perStateKeepLimit = std::max(1, std::min(8, context.config->path_search.max_candidate_face_hits));
        std::vector<PathState> acceptedCandidates;

        const ExpanderResult reflectionResult = ExpandReflection(context, currentState);
        AccumulateFailureReasons(result, reflectionResult);
        AppendCandidates(acceptedCandidates, reflectionResult, result, perExpanderKeepLimit);

        const ExpanderResult transmissionResult = ExpandTransmission(context, currentState);
        AccumulateFailureReasons(result, transmissionResult);
        AppendCandidates(acceptedCandidates, transmissionResult, result, perExpanderKeepLimit);

        const ExpanderResult diffractionResult = ExpandDiffraction(context, currentState);
        AccumulateFailureReasons(result, diffractionResult);
        AppendCandidates(acceptedCandidates, diffractionResult, result, perExpanderKeepLimit);

        SortAndTruncateCandidates(acceptedCandidates, perStateKeepLimit, result);

        std::ostringstream candidateTrace;
        candidateTrace << "Accepted candidate states after control: " << acceptedCandidates.size();
        result.trace_lines.push_back(candidateTrace.str());

        for (const PathState& nextState : acceptedCandidates)
        {
            if (nextState.has_reflection && nextState.has_transmission && !nextState.has_diffraction)
            {
                ++result.mixed_path_generated_count;
            }
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

    auto findCount = [&result](GeometryValidityReason reason) -> int
    {
        const auto it = result.failure_reason_counts.find(static_cast<int>(reason));
        return it == result.failure_reason_counts.end() ? 0 : it->second;
    };
    result.invalid_sequence_rejected_count = findCount(GeometryValidityReason::InvalidPathSequence) +
                                             findCount(GeometryValidityReason::DuplicateInteractionLoop);
    result.mixed_path_blocked_count = findCount(GeometryValidityReason::MixedPathNotAllowed);
    return result;
}

} // namespace rt
