#include "StateSignatureBuilder.h"

namespace rt {

namespace {

/// <summary>
/// Standard hash-combine primitive (boost::hash_combine style) for building
/// incremental 64-bit hash signatures from component fields.
/// </summary>
inline uint64_t HashCombine(uint64_t seed, uint64_t value) {
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

} // namespace

/// <summary>
/// Produces a 64-bit deduplication hash for a PathState by combining the most
/// recent interaction type, face/wedge id, medium id, depth, and budget counters.
/// Used to prune duplicate search states in the priority queue.
/// </summary>
uint64_t BuildStateSignature(const PathState& state, const AppConfig& config)
{
    uint64_t h = 0;
    h = HashCombine(h, static_cast<uint64_t>(state.last_interaction_type));
    h = HashCombine(h, static_cast<uint64_t>(state.last_hit_face_id));
    h = HashCombine(h, static_cast<uint64_t>(state.last_hit_wedge_id));
    h = HashCombine(h, static_cast<uint64_t>(state.current_medium_id));
    h = HashCombine(h, static_cast<uint64_t>(state.path_depth));
    h = HashCombine(h, static_cast<uint64_t>(state.remaining_reflections));
    h = HashCombine(h, static_cast<uint64_t>(state.remaining_transmissions));
    h = HashCombine(h, static_cast<uint64_t>(state.remaining_diffractions));
    h = HashCombine(h, static_cast<uint64_t>(state.remaining_total_expansions));
    return h;
}

} // namespace rt
