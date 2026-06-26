// v11: SBR diffraction tracer implementation.
#include "SbrDiffractionTracer.h"
#include "../common/math/MathConstants.h"
#include <cmath>

namespace rt {

bool SbrAnalyticalFermatPoint(const Point3& tx,
                              const Point3& rx,
                              const Point3& edgeStart,
                              const Point3& edgeEnd,
                              Point3& diffPoint,
                              double& edgeT) {
    const Vec3 edge = Subtract(edgeEnd, edgeStart);
    const double edgeLen2 = Dot(edge, edge);
    if (edgeLen2 < 1.0e-12) return false;

    const double tTx = Dot(Subtract(tx, edgeStart), edge) / edgeLen2;
    const Point3 txProjection = Add(edgeStart, Scale(edge, tTx));
    const double h1 = Length(Subtract(tx, txProjection));

    const double tRx = Dot(Subtract(rx, edgeStart), edge) / edgeLen2;
    const Point3 rxProjection = Add(edgeStart, Scale(edge, tRx));
    const double h2 = Length(Subtract(rx, rxProjection));

    const double hSum = h1 + h2;
    if (hSum < 1.0e-12) {
        diffPoint = rxProjection;
        edgeT = tRx;
        return true;
    }

    diffPoint = Add(rxProjection, Scale(Subtract(txProjection, rxProjection), h2 / hSum));
    edgeT = Dot(Subtract(diffPoint, edgeStart), edge) / edgeLen2;
    return edgeT >= 0.0 && edgeT <= 1.0;
}

bool SbrPointOnLargeWedgeSide(const Scene& scene, const Wedge& wedge, const Point3& p) {
    if (wedge.wedge_angle_deg < 183.0 || wedge.wedge_angle_deg > 357.0) {
        return false;
    }
    if (wedge.positive_face_id < 0 || wedge.negative_face_id < 0 ||
        wedge.positive_face_id >= static_cast<int>(scene.faces.size()) ||
        wedge.negative_face_id >= static_cast<int>(scene.faces.size())) {
        return true;
    }

    const Vec3& n0 = scene.faces[wedge.positive_face_id].normal;
    const Vec3& n1 = scene.faces[wedge.negative_face_id].normal;
    const double d0 = Dot(Subtract(p, wedge.center_point), n0);
    const double d1 = Dot(Subtract(p, wedge.center_point), n1);
    return !(d0 > 0.0 && d1 > 0.0);
}

void TracePointToPointDiffraction(const Scene& scene,
                                  const AppConfig& config,
                                  const Point3& tx,
                                  const Point3& rx,
                                  int rxIndex,
                                  const SceneQuery& query,
                                  std::vector<GeometricPath>& out) {
    if (config.sbr.max_diffraction_count <= 0 || scene.wedges.empty()) return;

    for (const Wedge& wedge : scene.wedges) {
        if (!wedge.diffractable) continue;
        if (!SbrPointOnLargeWedgeSide(scene, wedge, tx) ||
            !SbrPointOnLargeWedgeSide(scene, wedge, rx)) {
            continue;
        }

        Point3 diffPoint;
        double edgeT = 0.0;
        if (!SbrAnalyticalFermatPoint(tx, rx, wedge.segment_start, wedge.segment_end, diffPoint, edgeT)) {
            continue;
        }

        const double s1 = Length(Subtract(diffPoint, tx));
        const double s2 = Length(Subtract(rx, diffPoint));
        if (s1 < 0.01 || s2 < 0.01) continue;

        VisibilityQueryContext vqc;
        vqc.ignored_face_id = wedge.positive_face_id;
        vqc.ignored_face_id2 = wedge.negative_face_id;
        vqc.origin_offset_distance = 1.0e-3;
        vqc.target_shrink_distance = 1.0e-3;
        if (!query.IsVisible(tx, diffPoint, vqc)) continue;
        if (!query.IsVisible(diffPoint, rx, vqc)) continue;

        const Vec3 inDir = Normalize(Subtract(diffPoint, tx));
        const Vec3 outDir = Normalize(Subtract(rx, diffPoint));
        const Vec3 wedgeDir = Normalize(wedge.direction);

        GeometricPath path;
        path.path_id = -1;
        path.is_los = false;
        path.contains_transmission = false;
        path.valid = true;

        PathNode txNode;
        txNode.interaction_type = InteractionType::Tx;
        txNode.point = tx;
        txNode.direction = inDir;
        txNode.valid = true;
        path.nodes.push_back(txNode);

        PathNode dNode;
        dNode.interaction_type = InteractionType::Diffraction;
        dNode.point = diffPoint;
        dNode.wedge_id = wedge.wedge_id;
        dNode.surface_normal = wedgeDir;
        dNode.incident_direction = inDir;
        dNode.direction = outDir;
        dNode.segment_length_from_previous = s1;
        dNode.diffraction_diag.edge_parameter_t = edgeT;
        dNode.diffraction_diag.s1 = s1;
        dNode.diffraction_diag.s2 = s2;
        dNode.diffraction_diag.visibility_from_source = true;
        dNode.diffraction_diag.visibility_to_rx = true;
        dNode.diffraction_diag.fermat_endpoint_warning = edgeT < 1.0e-4 || edgeT > 1.0 - 1.0e-4;
        dNode.diffraction_diag.keller_residual = std::fabs(Dot(inDir, wedgeDir) - Dot(outDir, wedgeDir));
        dNode.valid = true;
        path.nodes.push_back(dNode);

        PathNode rxNode;
        rxNode.interaction_type = InteractionType::Rx;
        rxNode.point = rx;
        rxNode.incident_direction = outDir;
        rxNode.segment_length_from_previous = s2;
        rxNode.valid = true;
        path.nodes.push_back(rxNode);

        path.total_length = s1 + s2;
        out.push_back(std::move(path));
        (void)rxIndex;
    }
}

} // namespace rt
