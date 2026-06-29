// v11: SBR diffraction tracer — analytical Fermat point + Tx-D-Rx diffraction paths.
#pragma once

#include "../common/math/Vec3.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"
#include "../query/SceneQuery.h"
#include "../common/config/AppConfig.h"
#include <vector>

namespace rt {

/// Analytical Fermat diffraction point on an edge (weighted by Tx/Rx distances).
/// Returns true if the optimal point lies within the edge segment [0,1].
bool SbrAnalyticalFermatPoint(const Point3& tx, const Point3& rx,
                              const Point3& edgeStart, const Point3& edgeEnd,
                              Point3& diffPoint, double& edgeT);

/// Check that a point is in the propagation opening selected by the two
/// outward-facing wedge planes. Convex exterior wedges are the default.
bool SbrPointOnLargeWedgeSide(const Scene& scene, const Wedge& wedge, const Point3& p);

/// Enumerate independent Tx-D-Rx diffraction paths for one receiver.
/// Only generates single-diffraction paths (no R/T + D mixing).
void TracePointToPointDiffraction(const Scene& scene, const AppConfig& config,
                                  const Point3& tx, const Point3& rx, int rxIndex,
                                  const SceneQuery& query, std::vector<GeometricPath>& out);

} // namespace rt
