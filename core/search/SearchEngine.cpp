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
#include "../common/math/Vec3.h"

#include <cmath>
#include <algorithm>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace rt {

namespace {

struct ScoredState {
    PathState state;
    double priority = 0.0;
};

struct CompareScoredState {
    bool operator()(const ScoredState& a, const ScoredState& b) const {
        return a.priority > b.priority;  // min-heap: lower priority = explored first
    }
};

/// <summary>
/// Heuristic priority for best-first state expansion: combines estimated total
/// length (accumulated + Euclidean-to-Rx) with a depth bonus favouring deeper paths.
/// </summary>
double ComputeStatePriority(const PathState& state, const Point3& rx)
{
    double distToRx = Length(Subtract(rx, state.current_point));
    double estimatedTotal = state.accumulated_length + distToRx;
    double depthBonus = -state.path_depth * 0.5;
    return estimatedTotal + depthBonus;
}

/// <summary>
/// Composite candidate score used for sorting and truncation: penalises depth,
/// mechanism switches, and accumulated length to prefer simpler paths.
/// </summary>
double ComputeCandidateScore(const PathState& state)
{
    double score = 0.0;
    score += static_cast<double>(state.path_depth) * 100.0;
    score += static_cast<double>(state.mechanism_switch_count) * 10.0;
    score += state.accumulated_length * 0.01;
    return score;
}

/// <summary>
/// Sorts candidates by composite score in ascending order and truncates to
/// keepLimit, recording the discard count in result.
/// </summary>
void SortAndTruncateCandidates(std::vector<PathState>& candidates, int keepLimit, SearchEngineResult& result)
{
    std::sort(candidates.begin(), candidates.end(),
        [](const PathState& lhs, const PathState& rhs)
        {
            return ComputeCandidateScore(lhs) < ComputeCandidateScore(rhs);
        });

    if (keepLimit > 0 && static_cast<int>(candidates.size()) > keepLimit)
    {
        result.truncated_candidate_count += static_cast<int>(candidates.size()) - keepLimit;
        candidates.resize(static_cast<std::size_t>(keepLimit));
    }
}

/// <summary>
/// Merges an expander's next-state candidates into the accepted-candidates
/// vector after applying per-expander sorting and truncation.
/// </summary>
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

/// <summary>
/// Builds a FaceQueryContext for ray-casting from the current state, applying
/// self-hit avoidance via ignored face/object ids and the numeric tolerance origin-offset.
/// </summary>
FaceQueryContext BuildFaceQueryContext(const PathState& state, const AppConfig& config)
{
    FaceQueryContext context;
    context.ignored_face_id = state.ignored_face_id;
    context.ignored_object_id = state.ignored_object_id;
    context.ignore_origin_self_hit = true;
    context.origin_ignore_distance = config.numeric_tolerance.self_hit_ignore_distance;
    return context;
}

/// <summary>
/// Builds a VisibilityQueryContext for line-of-sight checks, with origin/target
/// offsets and attached-face ignore flags to avoid self-occlusion artefacts.
/// </summary>
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

/// <summary>
/// Creates the root PathState seeded at the transmitter with direction toward
/// the receiver, initialising all budget counters from the search configuration.
/// </summary>
PathState BuildInitialState(const PathSearchContext& context)
{
    PathState state;
    state.current_point = context.tx_point;
    state.current_direction = SafeNormalize(Subtract(context.rx_point, context.tx_point));
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

/// <summary>
/// Attempts to close the current search state with a final LOS leg to the
/// receiver. Returns true and populates path/traceLine if geometry and
/// visibility checks pass.
/// </summary>
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
    path.path_id = -1;  // assigned later when accepted into path_set
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

/// <summary>
/// Accumulates per-expander failure-reason counts into the global result
/// histogram for diagnostic traceability.
/// </summary>
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

    std::priority_queue<ScoredState, std::vector<ScoredState>, CompareScoredState> queue;
    std::unordered_set<uint64_t> stateSignatures;
    std::unordered_set<uint64_t> pathSignatures;

    PathState initialState = BuildInitialState(context);
    initialState.state_signature = BuildStateSignature(initialState, *context.config);
    stateSignatures.insert(initialState.state_signature);
    queue.push({initialState, ComputeStatePriority(initialState, context.rx_point)});
    result.generated_state_count = 1;
    result.uses_real_scene_query = true;
    result.trace_lines.push_back("Initial PathState constructed.");

    while (!queue.empty())
    {
        ScoredState scored = queue.top();
        queue.pop();
        PathState currentState = scored.state;

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

        if (context.config->path_search.enable_los)
        {
            GeometricPath path;
            std::string losTrace;
            if (TryBuildLosPath(context, currentState, path, losTrace))
            {
                path.path_signature = BuildPathSignature(path, *context.config);
                if (pathSignatures.insert(path.path_signature).second)
                {
                    path.path_id = static_cast<int>(result.path_set.paths.size());
                    result.path_set.paths.push_back(path);
                    result.trace_lines.push_back(losTrace);
                }
                else
                {
                    ++result.deduplicated_path_count;
                }
            }
            else
            {
                result.trace_lines.push_back(losTrace);
            }
        }

        if (currentState.remaining_total_expansions <= 0)
        {
            continue;
        }

        const int perExpanderKeepLimit = 8;
        const int perStateKeepLimit = 16;

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

        for (const PathState& nextState : acceptedCandidates)
        {
            if (nextState.has_reflection && nextState.has_transmission && !nextState.has_diffraction)
            {
                ++result.mixed_path_generated_count;
            }
            if (stateSignatures.insert(nextState.state_signature).second)
            {
                double pri = ComputeStatePriority(nextState, context.rx_point);
                queue.push({nextState, pri});
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
