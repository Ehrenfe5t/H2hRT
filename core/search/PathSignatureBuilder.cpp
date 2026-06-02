#include "PathSignatureBuilder.h"

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

inline int64_t Quantize(double value, double step) {
    return static_cast<int64_t>(std::floor(value / step + 0.5));
}

inline uint64_t HashInt64(int64_t v) {
    return static_cast<uint64_t>(v);
}

} // namespace

/// <summary>
/// v9 step10: 增强路径签名 — 加入量化节点坐标、段长和object_id，
/// 防止同一面元序列但交互点不同/不同物体的路径被误合并。
/// </summary>
uint64_t BuildPathSignature(const GeometricPath& path, const AppConfig& config)
{
    uint64_t h = 0;
    h = HashCombine(h, path.is_los ? 1ULL : 0ULL);
    h = HashCombine(h, static_cast<uint64_t>(path.nodes.size()));
    h = HashCombine(h, HashInt64(Quantize(path.total_length, 1.0e-3))); // v9: 路径总长1mm分桶

    const double posStep = config.numeric_tolerance.eps_deduplicate;

    for (const PathNode& node : path.nodes) {
        h = HashCombine(h, static_cast<uint64_t>(node.interaction_type));
        h = HashCombine(h, static_cast<uint64_t>(node.face_id));
        h = HashCombine(h, static_cast<uint64_t>(node.wedge_id));
        // v9 step10: 量化交互点坐标 — 区分同面不同位置
        h = HashCombine(h, HashInt64(Quantize(node.point.x, posStep)));
        h = HashCombine(h, HashInt64(Quantize(node.point.y, posStep)));
        h = HashCombine(h, HashInt64(Quantize(node.point.z, posStep)));
        // v9 step10: object_id — 区分不同物体
        h = HashCombine(h, static_cast<uint64_t>(node.object_id));
        // 透射路径需区分介质侧
        if (node.interaction_type == InteractionType::Transmission) {
            h = HashCombine(h, static_cast<uint64_t>(node.medium_in_id));
            h = HashCombine(h, static_cast<uint64_t>(node.medium_out_id));
        }
    }
    return h;
}

} // namespace rt
