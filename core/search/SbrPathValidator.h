// v11: SBR path validation — EM-ready checks and geometry residual evaluation.
#pragma once

#include "../path/GeometricPath.h"
#include "../scene/Scene.h"
#include <string>

namespace rt {

/// Check whether a path contains any transmission node.
bool SbrPathContainsTransmission(const GeometricPath& path);

/// Count interaction nodes of a given type.
int CountInteractionNodes(const GeometricPath& path, InteractionType type);

/// Validate that a geometric path is ready for EM consumption.
/// Returns false and sets *reason on failure.
bool ValidateSbrPathForEmReady(const GeometricPath& path, const Scene& scene, std::string* reason);

/// Compute geometry residual (reflection Snell / transmission Snell / Keller cone) for a path.
void EvaluatePathGeometryResidual(GeometricPath& path);

} // namespace rt
