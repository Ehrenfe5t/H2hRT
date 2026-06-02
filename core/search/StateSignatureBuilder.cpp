#include "StateSignatureBuilder.h"

#include "../common/math/Vec3.h"
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Standard hash-combine primitive (boost::hash_combine style) for building
/// incremental 64-bit hash signatures from component fields.
/// </summary>
inline uint64_t HashCombine(uint64_t seed, uint64_t value) {
    return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

/// <summary>
/// Quantize a double to an integer bucket using the given step size.
/// Points within step/2 of each other map to the same bucket.
/// </summary>
inline int64_t Quantize(double value, double step) {
    return static_cast<int64_t>(std::floor(value / step + 0.5));
}

inline uint64_t HashInt64(int64_t v) {
    return static_cast<uint64_t>(v);
}

} // namespace

/// <summary>
/// v9 step9: 增强状态签名 — 加入量化坐标、方向和累计长度，
/// 防止同一面元上不同交互点的路径被误去重。
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

    // v9 step9: 量化坐标和方向 — eps_deduplicate默认1e-5,
    // 同一点1μm内视为同一状态; 方向量化到1e-3 (≈0.057°)
    const double posStep = config.numeric_tolerance.eps_deduplicate;
    const double dirStep = 1.0e-3; // ~0.057° angular resolution for 64-bit hash

    h = HashCombine(h, HashInt64(Quantize(state.current_point.x, posStep)));
    h = HashCombine(h, HashInt64(Quantize(state.current_point.y, posStep)));
    h = HashCombine(h, HashInt64(Quantize(state.current_point.z, posStep)));

    h = HashCombine(h, HashInt64(Quantize(state.current_direction.x, dirStep)));
    h = HashCombine(h, HashInt64(Quantize(state.current_direction.y, dirStep)));
    h = HashCombine(h, HashInt64(Quantize(state.current_direction.z, dirStep)));

    // 累计长度分桶 (每1mm一桶)
    const double lenStep = 1.0e-3;
    h = HashCombine(h, HashInt64(Quantize(state.accumulated_length, lenStep)));

    // 节点数 (区分不同路径拓扑)
    h = HashCombine(h, static_cast<uint64_t>(state.traversed_nodes.size()));

    return h;
}

} // namespace rt
