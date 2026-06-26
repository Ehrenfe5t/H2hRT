// v11: SBR path deduplication — strict signature, exact equality, similarity key, post-process.
// Migrated from independent SBR module. Used by SbrEngine::RunPointToPoint and legacy coverage paths.
#pragma once

#include "../path/GeometricPath.h"
#include "../common/config/AppConfig.h"
#include <cstdint>
#include <vector>

namespace rt {

/// Power proxy score for path sorting (geometric heuristic, not EM).
double SbrPathPowerScore(const GeometricPath& path);

/// Quantize a double value by step size.
int64_t SbrQuantize(double value, double step);

/// Build strict path signature (FNV-1a 64-bit hash with quantized coords).
uint64_t SbrBuildPathSignature(const GeometricPath& path, double lengthTolM, double pointTolM);

/// Exact path equality check (field-by-field comparison).
bool SbrPathExactEqual(const GeometricPath& a, const GeometricPath& b, double lengthTolM, double pointTolM);

/// Build similarity key for near-equivalent path compression (coverage only).
uint64_t SbrBuildSimilarityKey(const GeometricPath& path, double lengthTolM, double pointTolM);

/// Full post-process pipeline: strict dedup → similarity pruning → top-N → renumber.
void SbrPostProcess(std::vector<GeometricPath>& paths, const AppConfig& config);

} // namespace rt
