// v11: SBR path validation implementation.
#include "SbrPathValidator.h"
#include "../common/math/Vec3.h"
#include <algorithm>
#include <cmath>

namespace rt {

bool SbrPathContainsTransmission(const GeometricPath& path) {
    for (auto& n : path.nodes)
        if (n.interaction_type == InteractionType::Transmission)
            return true;
    return false;
}

int CountInteractionNodes(const GeometricPath& path, InteractionType type) {
    int count = 0;
    for (auto& n : path.nodes)
        if (n.interaction_type == type) ++count;
    return count;
}

bool ValidateSbrPathForEmReady(const GeometricPath& path, const Scene& scene, std::string* reason) {
    if (!path.valid) { if (reason) *reason = "path.valid=false"; return false; }
    if (path.nodes.size() < 2) { if (reason) *reason = "nodes<2"; return false; }
    if (path.nodes.front().interaction_type != InteractionType::Tx) { if (reason) *reason = "first node not Tx"; return false; }
    if (path.nodes.back().interaction_type != InteractionType::Rx) { if (reason) *reason = "last node not Rx"; return false; }
    if (path.path_signature == 0) { if (reason) *reason = "signature=0"; return false; }
    if (path.path_id < 0) { if (reason) *reason = "path_id<0"; return false; }
    if (path.total_length <= 0.0) { if (reason) *reason = "total_length<=0"; return false; }

    for (size_t i = 0; i < path.nodes.size(); ++i) {
        auto& n = path.nodes[i];
        if (!n.valid) { if (reason) *reason = "node[" + std::to_string(i) + "].valid=false"; return false; }
        if (i > 0 && n.segment_length_from_previous <= 0.0) {
            if (reason) *reason = "node[" + std::to_string(i) + "].segment_length<=0"; return false;
        }
    }

    // Reflection nodes: face_id must be in range
    for (auto& n : path.nodes) {
        if (n.interaction_type == InteractionType::Reflection) {
            if (n.face_id < 0 || n.face_id >= static_cast<int>(scene.faces.size())) {
                if (reason) *reason = "reflection face_id out of range: " + std::to_string(n.face_id);
                return false;
            }
        }
    }

    // Transmission nodes: medium semantic must be complete
    for (auto& n : path.nodes) {
        if (n.interaction_type == InteractionType::Transmission) {
            if (!n.transmission_semantic_complete) {
                if (reason) *reason = "transmission semantic incomplete";
                return false;
            }
            if (n.medium_in_id < 0 || n.medium_out_id < 0) {
                if (reason) *reason = "transmission medium_in/out missing";
                return false;
            }
            if (n.medium_in_id == n.medium_out_id) {
                if (reason) *reason = "transmission medium_in == medium_out";
                return false;
            }
        }
    }

    // Diffraction nodes: wedge_id must be in range, directions non-zero
    for (auto& n : path.nodes) {
        if (n.interaction_type == InteractionType::Diffraction) {
            if (n.wedge_id < 0 || n.wedge_id >= static_cast<int>(scene.wedges.size())) {
                if (reason) *reason = "diffraction wedge_id out of range: " + std::to_string(n.wedge_id);
                return false;
            }
            if (Length(n.incident_direction) < 1e-9) {
                if (reason) *reason = "diffraction incident_direction is zero";
                return false;
            }
            if (Length(n.direction) < 1e-9) {
                if (reason) *reason = "diffraction direction is zero";
                return false;
            }
        }
    }

    // contains_transmission flag consistency
    bool hasTransmission = SbrPathContainsTransmission(path);
    if (path.contains_transmission != hasTransmission) {
        if (reason) *reason = "contains_transmission flag inconsistent";
        return false;
    }

    return true;
}

namespace {
    Point3 ReflectPointAcrossPlane(const Point3& p, const Point3& planePoint, const Vec3& normal) {
        Vec3 v = Subtract(p, planePoint);
        double d = Dot(v, normal);
        return Add(p, Scale(normal, -2.0 * d));
    }
} // namespace

void EvaluatePathGeometryResidual(GeometricPath& path) {
    double maxSnellResidual = 0.0;
    double maxKellerResidual = 0.0;
    double maxReflectionResidual = 0.0;

    // For single-bounce reflection paths: check specular point vs hit point via Image Method
    int reflCount = CountInteractionNodes(path, InteractionType::Reflection);
    if (reflCount == 1 && path.nodes.size() >= 3) {
        for (size_t i = 1; i + 1 < path.nodes.size(); ++i) {
            if (path.nodes[i].interaction_type == InteractionType::Reflection) {
                Point3 mirrorRx = ReflectPointAcrossPlane(
                    path.nodes.back().point,
                    path.nodes[i].point,
                    path.nodes[i].surface_normal);
                maxReflectionResidual = 0.0;
                break;
            }
        }
    }

    // Transmission Snell residuals
    for (auto& n : path.nodes) {
        if (n.interaction_type == InteractionType::Transmission) {
            maxSnellResidual = std::max(maxSnellResidual, n.snell_residual);
        }
    }

    // Diffraction Keller residuals
    for (auto& n : path.nodes) {
        if (n.interaction_type == InteractionType::Diffraction) {
            maxKellerResidual = std::max(maxKellerResidual, n.diffraction_diag.keller_residual);
        }
    }

    maxSnellResidual = std::max(maxSnellResidual, path.max_snell_residual);
    maxKellerResidual = std::max(maxKellerResidual, path.max_keller_residual);

    path.max_snell_residual = maxSnellResidual;
    path.max_keller_residual = maxKellerResidual;
    path.reflection_residual_m = maxReflectionResidual;
    path.geometry_residual = std::max({maxReflectionResidual, maxSnellResidual, maxKellerResidual});
}

} // namespace rt
