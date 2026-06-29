// v11: SBR path deduplication implementation — migrated from independent SBR module.
#include "SbrPathDeduplicator.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace rt {

double SbrPathPowerScore(const GeometricPath& path) {
    double L = std::max(path.total_length, 1e-3);
    double s = -2.0 * std::log(L);
    for (auto& n : path.nodes) {
        switch (n.interaction_type) {
        case InteractionType::Reflection:    s += std::log(0.45); break;
        case InteractionType::Transmission:  s += std::log(0.35); break;
        case InteractionType::Diffraction:   s += std::log(0.08); break;
        default: break;
        }
    }
    return s;
}

int64_t SbrQuantize(double value, double step) {
    return static_cast<int64_t>(std::floor(value / step + 0.5));
}

uint64_t SbrBuildPathSignature(const GeometricPath& path, double lengthTolM, double pointTolM) {
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](uint64_t v) { h ^= v; h *= 1099511628211ull; };
    mix(path.is_los ? 1ull : 0ull);
    mix(static_cast<uint64_t>(path.nodes.size()));
    mix(static_cast<uint64_t>(SbrQuantize(path.total_length, lengthTolM)));
    for (auto& n : path.nodes) {
        mix(static_cast<uint64_t>(static_cast<int>(n.interaction_type) + 257));
        mix(static_cast<uint64_t>(n.face_id + 4099));
        mix(static_cast<uint64_t>(n.wedge_id + 8191));
        mix(static_cast<uint64_t>(n.object_id + 16381));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.x, pointTolM)));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.y, pointTolM)));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.z, pointTolM)));
        if (n.interaction_type == InteractionType::Transmission) {
            mix(static_cast<uint64_t>(n.medium_in_id + 32771));
            mix(static_cast<uint64_t>(n.medium_out_id + 65537));
        }
    }
    return h;
}

bool SbrPathExactEqual(const GeometricPath& a, const GeometricPath& b, double lengthTolM, double pointTolM) {
    if (a.nodes.size() != b.nodes.size() || a.is_los != b.is_los) return false;
    if (std::fabs(a.total_length - b.total_length) > lengthTolM) return false;
    for (size_t i = 0; i < a.nodes.size(); ++i) {
        auto& na = a.nodes[i];
        auto& nb = b.nodes[i];
        if (na.interaction_type != nb.interaction_type ||
            na.face_id != nb.face_id ||
            na.wedge_id != nb.wedge_id ||
            na.object_id != nb.object_id)
            return false;
        if (na.interaction_type == InteractionType::Transmission &&
            (na.medium_in_id != nb.medium_in_id || na.medium_out_id != nb.medium_out_id))
            return false;
        double dx = na.point.x - nb.point.x;
        double dy = na.point.y - nb.point.y;
        double dz = na.point.z - nb.point.z;
        if (std::sqrt(dx * dx + dy * dy + dz * dz) > pointTolM) return false;
    }
    return true;
}

uint64_t SbrBuildSimilarityKey(const GeometricPath& path, double lengthTolM, double pointTolM) {
    uint64_t h = 1469598103934665603ull;
    auto mix = [&](uint64_t v) { h ^= v; h *= 1099511628211ull; };
    mix(static_cast<uint64_t>(std::llround(path.total_length / std::max(lengthTolM, 0.01))));
    for (auto& n : path.nodes) {
        mix(static_cast<uint64_t>(static_cast<int>(n.interaction_type) + 257));
        if (n.face_id >= 0) mix(static_cast<uint64_t>(n.face_id + 4099));
        if (n.wedge_id >= 0) mix(static_cast<uint64_t>(n.wedge_id + 8191));
        if (n.object_id >= 0) mix(static_cast<uint64_t>(n.object_id + 16381));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.x, pointTolM)));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.y, pointTolM)));
        mix(static_cast<uint64_t>(SbrQuantize(n.point.z, pointTolM)));
    }
    return h;
}

void SbrPostProcess(std::vector<GeometricPath>& paths, const AppConfig& config) {
    if (paths.empty()) return;
    const double kLengthTol = 1e-3;
    const double kPointTol  = 1e-5;

    // T1: strict dedup
    if (config.sbr.enable_path_dedup) {
        std::unordered_map<uint64_t, std::vector<size_t>> buckets;
        for (size_t i = 0; i < paths.size(); ++i)
            buckets[SbrBuildPathSignature(paths[i], kLengthTol, kPointTol)].push_back(i);
        std::vector<GeometricPath> kept;
        for (auto& [sig, indices] : buckets) {
            for (size_t j = 0; j < indices.size(); ++j) {
                bool dup = false;
                for (size_t k = 0; k < j; ++k) {
                    if (SbrPathExactEqual(paths[indices[k]], paths[indices[j]], kLengthTol, kPointTol)) {
                        dup = true; break;
                    }
                }
                if (!dup) {
                    kept.push_back(std::move(paths[indices[j]]));
                } else {
                    for (GeometricPath& existing : kept) {
                        if (SbrPathExactEqual(existing, paths[indices[j]], kLengthTol, kPointTol)) {
                            existing.candidate_support_count += paths[indices[j]].candidate_support_count;
                            break;
                        }
                    }
                }
            }
        }
        paths = std::move(kept);
    }

    // Write back strict signatures
    for (auto& p : paths)
        p.path_signature = SbrBuildPathSignature(p, kLengthTol, kPointTol);

    // T2: similarity compression (coverage only, off by default for P2P)
    double pointCoarse = std::max(config.sbr.path_similarity_length_tol_m, 0.05);
    if (config.sbr.enable_path_similarity_pruning && paths.size() > 1) {
        std::unordered_map<uint64_t, size_t> bestIdx;
        for (size_t i = 0; i < paths.size(); ++i) {
            uint64_t key = SbrBuildSimilarityKey(paths[i], config.sbr.path_similarity_length_tol_m, pointCoarse);
            auto it = bestIdx.find(key);
            if (it == bestIdx.end() || SbrPathPowerScore(paths[i]) > SbrPathPowerScore(paths[it->second]))
                bestIdx[key] = i;
        }
        std::vector<GeometricPath> pruned;
        for (auto& kv : bestIdx)
            pruned.push_back(std::move(paths[kv.second]));
        paths = std::move(pruned);
        for (auto& p : paths)
            p.path_signature = SbrBuildPathSignature(p, kLengthTol, kPointTol);
    }

    // T3: top-N
    int topN = config.sbr.path_top_n_per_rx;
    if (topN > 0 && static_cast<int>(paths.size()) > topN) {
        std::sort(paths.begin(), paths.end(),
            [](auto& a, auto& b) { return SbrPathPowerScore(a) > SbrPathPowerScore(b); });
        paths.resize(topN);
    }

    // Renumber
    for (size_t i = 0; i < paths.size(); ++i)
        paths[i].path_id = static_cast<int>(i);
}

} // namespace rt
