#include "PathSignatureBuilder.h"

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
/// Produces a 64-bit deduplication hash for a completed GeometricPath by
/// combining the LOS flag, node count, and the interaction type / face id /
/// wedge id of every path node. Used to prune near-duplicate final paths.
/// </summary>
uint64_t BuildPathSignature(const GeometricPath& path, const AppConfig& config)
{
    uint64_t h = 0;
    h = HashCombine(h, path.is_los ? 1ULL : 0ULL);
    h = HashCombine(h, static_cast<uint64_t>(path.nodes.size()));
    for (const PathNode& node : path.nodes) {
        h = HashCombine(h, static_cast<uint64_t>(node.interaction_type));
        h = HashCombine(h, static_cast<uint64_t>(node.face_id));
        h = HashCombine(h, static_cast<uint64_t>(node.wedge_id));
    }
    return h;
}

} // namespace rt
