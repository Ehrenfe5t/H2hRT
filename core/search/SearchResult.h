#pragma once

#include "../path/GeometricPath.h"

#include <map>
#include <string>
#include <vector>

namespace rt {

// Geometry search result consumed by the current P2P SBR -> EM main chain.
struct SearchEngineResult {
    bool succeeded = false;
    GeometricPathSet path_set;
    std::string source_tag = "sbr_p2p_geometry_output";
    bool uses_real_scene_query = false;
    int control_rule_rejected_state_count = 0;
    int invalid_sequence_rejected_count = 0;
    int mixed_path_blocked_count = 0;
    int mixed_path_generated_count = 0;
    int candidate_state_count = 0;
    int accepted_state_count = 0;
    int truncated_candidate_count = 0;
    int generated_state_count = 0;
    int deduplicated_state_count = 0;
    int deduplicated_path_count = 0;
    std::map<int, int> failure_reason_counts;
    std::vector<std::string> trace_lines;
};

} // namespace rt
