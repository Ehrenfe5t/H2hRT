#include "SbrPathRefiner.h"

#include "SbrPathDeduplicator.h"
#include "SbrPathValidator.h"
#include "../common/math/Vec3.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <unordered_map>

namespace rt {
namespace {

constexpr double kPlaneToleranceM = 1.0e-6;
constexpr double kPatchToleranceM = 2.0e-5;
constexpr double kDirectionTolerance = 2.0e-5;

struct DisjointSet {
    explicit DisjointSet(int count) : parent(count), rank(count, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }
    int Find(int x) {
        if (parent[x] != x) parent[x] = Find(parent[x]);
        return parent[x];
    }
    void Join(int a, int b) {
        a = Find(a); b = Find(b);
        if (a == b) return;
        if (rank[a] < rank[b]) std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b]) ++rank[a];
    }
    std::vector<int> parent;
    std::vector<int> rank;
};

bool ValidFaceId(const Scene& scene, int faceId) {
    return faceId >= 0 && faceId < static_cast<int>(scene.faces.size());
}

bool SamePatchSemantics(const Face& a, const Face& b) {
    if (a.object_id != b.object_id || a.surface_material_id != b.surface_material_id ||
        a.front_material_id != b.front_material_id || a.back_material_id != b.back_material_id ||
        a.front_medium_id != b.front_medium_id || a.back_medium_id != b.back_medium_id ||
        a.propagation_flags != b.propagation_flags) return false;
    const Vec3 na = Normalize(a.normal);
    const Vec3 nb = Normalize(b.normal);
    if (Dot(na, nb) < 1.0 - 1.0e-9) return false;
    return std::fabs(Dot(Subtract(b.centroid, a.centroid), na)) <= kPlaneToleranceM;
}

Point3 ReflectPoint(const Point3& point, const Point3& planePoint, const Vec3& normal) {
    return Add(point, Scale(normal, -2.0 * Dot(Subtract(point, planePoint), normal)));
}

bool PointInTriangle(const Point3& p, const Point3& a, const Point3& b, const Point3& c, double tol) {
    const Vec3 v0 = Subtract(b, a);
    const Vec3 v1 = Subtract(c, a);
    const Vec3 v2 = Subtract(p, a);
    const Vec3 n = Cross(v0, v1);
    const double nLen = Length(n);
    if (nLen <= 1.0e-15 || std::fabs(Dot(v2, Scale(n, 1.0 / nLen))) > tol) return false;
    const double d00 = Dot(v0, v0), d01 = Dot(v0, v1), d11 = Dot(v1, v1);
    const double d20 = Dot(v2, v0), d21 = Dot(v2, v1);
    const double denom = d00 * d11 - d01 * d01;
    if (std::fabs(denom) <= 1.0e-20) return false;
    const double v = (d11 * d20 - d01 * d21) / denom;
    const double w = (d00 * d21 - d01 * d20) / denom;
    const double u = 1.0 - v - w;
    const double edgeTol = tol / std::max({Length(v0), Length(v1), Length(Subtract(c, b)), 1.0});
    return u >= -edgeTol && v >= -edgeTol && w >= -edgeTol;
}

int FindFaceOnPatch(const Scene& scene, const std::vector<int>& faceToPatch, int patchId, const Point3& p) {
    for (int faceId = 0; faceId < static_cast<int>(scene.faces.size()); ++faceId) {
        if (faceId >= static_cast<int>(faceToPatch.size()) || faceToPatch[faceId] != patchId) continue;
        const Face& f = scene.faces[faceId];
        if (f.vertex_index0 < 0 || f.vertex_index1 < 0 || f.vertex_index2 < 0 ||
            f.vertex_index0 >= static_cast<int>(scene.vertices.size()) ||
            f.vertex_index1 >= static_cast<int>(scene.vertices.size()) ||
            f.vertex_index2 >= static_cast<int>(scene.vertices.size())) continue;
        if (PointInTriangle(p, scene.vertices[f.vertex_index0], scene.vertices[f.vertex_index1],
                            scene.vertices[f.vertex_index2], kPatchToleranceM)) return faceId;
    }
    return -1;
}

void BuildPlaneBasis(const Face& face, Vec3& tangent, Vec3& bitangent) {
    const Vec3 n = Normalize(face.normal);
    tangent = face.local_frame.valid ? Normalize(face.local_frame.tangent) : Vec3{};
    if (Length(tangent) < 0.5 || std::fabs(Dot(tangent, n)) > 1.0e-6) {
        const Vec3 seed = std::fabs(n.z) < 0.9 ? MakeVec3(0.0, 0.0, 1.0) : MakeVec3(0.0, 1.0, 0.0);
        tangent = Normalize(Cross(seed, n));
    }
    bitangent = Normalize(Cross(n, tangent));
}

double RefractiveIndex(const MaterialDatabase* db, const std::string& name, double frequencyHz) {
    if (!db || db->empty() || name.empty()) return 1.0;
    return std::sqrt(std::max(1.0, db->QueryByName(name, frequencyHz).epsilon_r));
}

void InterfaceIndices(const Face& face, const Vec3& incident, const MaterialDatabase* db,
                      double frequencyHz, double& n1, double& n2, bool& fromFront) {
    fromFront = Dot(incident, face.normal) < 0.0;
    n1 = RefractiveIndex(db, fromFront ? face.front_material_name : face.back_material_name, frequencyHz);
    n2 = RefractiveIndex(db, fromFront ? face.back_material_name : face.front_material_name, frequencyHz);
}

Vec3 ExpectedDirection(InteractionType type, const Vec3& incident, const Face& face,
                       const MaterialDatabase* db, double frequencyHz, bool* tir = nullptr) {
    if (tir) *tir = false;
    if (type == InteractionType::Reflection) return Normalize(Reflect(incident, face.normal));
    if (type == InteractionType::Transmission) {
        double n1 = 1.0, n2 = 1.0; bool fromFront = true;
        InterfaceIndices(face, incident, db, frequencyHz, n1, n2, fromFront);
        const SnellResult sr = SnellRefractV2(incident, face.normal, n1, n2);
        if (!sr.valid || sr.total_internal_reflection) {
            if (tir) *tir = true;
            return Vec3{};
        }
        return Normalize(sr.direction);
    }
    return Vec3{};
}

std::vector<double> InteractionResiduals(const std::vector<Point3>& points,
                                         const std::vector<const Face*>& faces,
                                         const std::vector<InteractionType>& types,
                                         const MaterialDatabase* db, double frequencyHz,
                                         bool* valid) {
    std::vector<double> residuals;
    residuals.reserve(types.size() * 3);
    *valid = true;
    for (std::size_t i = 0; i < types.size(); ++i) {
        const Vec3 incoming = Normalize(Subtract(points[i + 1], points[i]));
        const Vec3 outgoing = Normalize(Subtract(points[i + 2], points[i + 1]));
        bool tir = false;
        const Vec3 expected = ExpectedDirection(types[i], incoming, *faces[i], db, frequencyHz, &tir);
        if (tir || Length(incoming) < 0.5 || Length(outgoing) < 0.5) {
            *valid = false;
            return {};
        }
        const Vec3 delta = Subtract(outgoing, expected);
        residuals.push_back(delta.x);
        residuals.push_back(delta.y);
        residuals.push_back(delta.z);
    }
    return residuals;
}

double SquaredNorm(const std::vector<double>& values) {
    double sum = 0.0;
    for (double v : values) sum += v * v;
    return sum;
}

bool SolveLinearSystem(std::vector<std::vector<double>> a, std::vector<double> b, std::vector<double>& x) {
    const int n = static_cast<int>(b.size());
    for (int col = 0; col < n; ++col) {
        int pivot = col;
        for (int row = col + 1; row < n; ++row)
            if (std::fabs(a[row][col]) > std::fabs(a[pivot][col])) pivot = row;
        if (std::fabs(a[pivot][col]) < 1.0e-14) return false;
        std::swap(a[pivot], a[col]);
        std::swap(b[pivot], b[col]);
        const double inv = 1.0 / a[col][col];
        for (int j = col; j < n; ++j) a[col][j] *= inv;
        b[col] *= inv;
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            const double factor = a[row][col];
            for (int j = col; j < n; ++j) a[row][j] -= factor * a[col][j];
            b[row] -= factor * b[col];
        }
    }
    x = std::move(b);
    return true;
}

bool RefineByImageMethod(GeometricPath& path, const Scene& scene,
                         const std::vector<int>& faceToPatch, std::string* reason) {
    std::vector<std::size_t> interactionIndices;
    std::vector<Point3> images(1, path.nodes.front().point);
    for (std::size_t i = 1; i + 1 < path.nodes.size(); ++i) {
        if (path.nodes[i].interaction_type != InteractionType::Reflection) {
            if (reason) *reason = "image method received non-reflection interaction";
            return false;
        }
        if (!ValidFaceId(scene, path.nodes[i].face_id)) {
            if (reason) *reason = "reflection face id out of range";
            return false;
        }
        interactionIndices.push_back(i);
        const Face& face = scene.faces[path.nodes[i].face_id];
        images.push_back(ReflectPoint(images.back(), face.centroid, Normalize(face.normal)));
    }
    Point3 current = path.nodes.back().point;
    for (int j = static_cast<int>(interactionIndices.size()) - 1; j >= 0; --j) {
        PathNode& node = path.nodes[interactionIndices[j]];
        const Face& face = scene.faces[node.face_id];
        const Vec3 towardImage = Subtract(images[j + 1], current);
        const double denom = Dot(towardImage, Normalize(face.normal));
        if (std::fabs(denom) < 1.0e-12) {
            if (reason) *reason = "image line is parallel to reflection plane";
            return false;
        }
        const double t = Dot(Subtract(face.centroid, current), Normalize(face.normal)) / denom;
        if (t <= 1.0e-9 || t >= 1.0 - 1.0e-9) {
            if (reason) *reason = "image intersection lies outside propagation segment";
            return false;
        }
        node.point = Add(current, Scale(towardImage, t));
        const int patchId = faceToPatch[node.face_id];
        const int containingFace = FindFaceOnPatch(scene, faceToPatch, patchId, node.point);
        if (containingFace < 0) {
            if (reason) *reason = "specular point lies outside physical surface patch";
            return false;
        }
        node.face_id = containingFace;
        node.surface_patch_id = patchId;
        current = node.point;
    }
    path.refinement_method = "image_method";
    path.refinement_iterations = 1;
    return true;
}

bool RefineByLevenbergMarquardt(GeometricPath& path, const Scene& scene,
                                const MaterialDatabase* db, double frequencyHz,
                                const std::vector<int>& faceToPatch, std::string* reason) {
    std::vector<std::size_t> indices;
    std::vector<const Face*> faces;
    std::vector<InteractionType> types;
    std::vector<Vec3> tangents, bitangents;
    std::vector<double> x;
    for (std::size_t i = 1; i + 1 < path.nodes.size(); ++i) {
        if (path.nodes[i].interaction_type != InteractionType::Reflection &&
            path.nodes[i].interaction_type != InteractionType::Transmission) {
            if (reason) *reason = "mixed optimizer supports reflection/transmission nodes only";
            return false;
        }
        if (!ValidFaceId(scene, path.nodes[i].face_id)) {
            if (reason) *reason = "interaction face id out of range";
            return false;
        }
        indices.push_back(i);
        faces.push_back(&scene.faces[path.nodes[i].face_id]);
        types.push_back(path.nodes[i].interaction_type);
        Vec3 t, b; BuildPlaneBasis(*faces.back(), t, b);
        tangents.push_back(t); bitangents.push_back(b);
        const Vec3 offset = Subtract(path.nodes[i].point, faces.back()->centroid);
        x.push_back(Dot(offset, t)); x.push_back(Dot(offset, b));
    }
    auto buildPoints = [&](const std::vector<double>& vars) {
        std::vector<Point3> points;
        points.reserve(indices.size() + 2);
        points.push_back(path.nodes.front().point);
        for (std::size_t i = 0; i < indices.size(); ++i) {
            Point3 p = Add(faces[i]->centroid, Scale(tangents[i], vars[2 * i]));
            p = Add(p, Scale(bitangents[i], vars[2 * i + 1]));
            points.push_back(p);
        }
        points.push_back(path.nodes.back().point);
        return points;
    };

    double lambda = 1.0e-3;
    int acceptedIterations = 0;
    for (int iteration = 0; iteration < 40; ++iteration) {
        bool valid = true;
        const std::vector<double> r = InteractionResiduals(buildPoints(x), faces, types, db, frequencyHz, &valid);
        if (!valid) { if (reason) *reason = "candidate requires total internal reflection"; return false; }
        const double cost = SquaredNorm(r);
        if (std::sqrt(cost / std::max<std::size_t>(1, r.size())) < 1.0e-9) break;
        const int m = static_cast<int>(r.size()), n = static_cast<int>(x.size());
        std::vector<std::vector<double>> jac(m, std::vector<double>(n, 0.0));
        for (int col = 0; col < n; ++col) {
            const double h = 1.0e-6 * std::max(1.0, std::fabs(x[col]));
            std::vector<double> xp = x, xm = x;
            xp[col] += h; xm[col] -= h;
            bool vp = true, vm = true;
            const auto rp = InteractionResiduals(buildPoints(xp), faces, types, db, frequencyHz, &vp);
            const auto rm = InteractionResiduals(buildPoints(xm), faces, types, db, frequencyHz, &vm);
            if (!vp || !vm) continue;
            for (int row = 0; row < m; ++row) jac[row][col] = (rp[row] - rm[row]) / (2.0 * h);
        }
        std::vector<std::vector<double>> normal(n, std::vector<double>(n, 0.0));
        std::vector<double> rhs(n, 0.0);
        for (int i = 0; i < n; ++i) {
            for (int row = 0; row < m; ++row) rhs[i] -= jac[row][i] * r[row];
            for (int j = 0; j < n; ++j)
                for (int row = 0; row < m; ++row) normal[i][j] += jac[row][i] * jac[row][j];
            normal[i][i] += lambda;
        }
        std::vector<double> delta;
        if (!SolveLinearSystem(normal, rhs, delta)) { lambda *= 10.0; continue; }
        std::vector<double> trial = x;
        for (int i = 0; i < n; ++i) trial[i] += delta[i];
        bool trialValid = true;
        const auto trialResidual = InteractionResiduals(buildPoints(trial), faces, types, db, frequencyHz, &trialValid);
        if (trialValid && SquaredNorm(trialResidual) < cost) {
            x = std::move(trial);
            lambda = std::max(1.0e-12, lambda * 0.3);
            ++acceptedIterations;
        } else {
            lambda = std::min(1.0e12, lambda * 10.0);
        }
    }
    const auto finalPoints = buildPoints(x);
    bool finalValid = true;
    const auto finalResidual = InteractionResiduals(finalPoints, faces, types, db, frequencyHz, &finalValid);
    if (!finalValid || std::sqrt(SquaredNorm(finalResidual) / std::max<std::size_t>(1, finalResidual.size())) > kDirectionTolerance) {
        if (reason) *reason = "reflection/Snell optimizer did not converge";
        return false;
    }
    for (std::size_t i = 0; i < indices.size(); ++i) {
        PathNode& node = path.nodes[indices[i]];
        const int patchId = faceToPatch[node.face_id];
        const int containingFace = FindFaceOnPatch(scene, faceToPatch, patchId, finalPoints[i + 1]);
        if (containingFace < 0) {
            if (reason) *reason = "optimized interaction lies outside physical surface patch";
            return false;
        }
        node.point = finalPoints[i + 1];
        node.face_id = containingFace;
        node.surface_patch_id = patchId;
    }
    path.refinement_method = "constrained_lm";
    path.refinement_iterations = acceptedIterations;
    return true;
}

void RecomputePathGeometry(GeometricPath& path, const Scene& scene, const MaterialDatabase* db, double frequencyHz) {
    path.total_length = 0.0;
    path.max_reflection_residual = 0.0;
    path.max_snell_residual = 0.0;
    for (std::size_t i = 0; i < path.nodes.size(); ++i) {
        PathNode& node = path.nodes[i];
        node.segment_length_from_previous = i == 0 ? 0.0 : Length(Subtract(node.point, path.nodes[i - 1].point));
        if (i > 0) {
            node.incident_direction = Normalize(Subtract(node.point, path.nodes[i - 1].point));
            path.total_length += node.segment_length_from_previous;
        }
        node.direction = i + 1 < path.nodes.size()
            ? Normalize(Subtract(path.nodes[i + 1].point, node.point)) : Vec3{};
        if ((node.interaction_type == InteractionType::Reflection ||
             node.interaction_type == InteractionType::Transmission) && ValidFaceId(scene, node.face_id)) {
            const Face& face = scene.faces[node.face_id];
            node.surface_normal = face.normal;
            node.object_id = face.object_id;
            bool tir = false;
            const Vec3 expected = ExpectedDirection(node.interaction_type, node.incident_direction,
                                                     face, db, frequencyHz, &tir);
            const double directionResidual = tir ? 1.0 : Length(Subtract(node.direction, expected));
            if (node.interaction_type == InteractionType::Reflection) {
                path.max_reflection_residual = std::max(path.max_reflection_residual, directionResidual);
            } else {
                double n1 = 1.0, n2 = 1.0; bool fromFront = true;
                InterfaceIndices(face, node.incident_direction, db, frequencyHz, n1, n2, fromFront);
                const double sinI = Length(Cross(node.incident_direction, Normalize(face.normal)));
                const double sinT = Length(Cross(node.direction, Normalize(face.normal)));
                node.snell_residual = std::fabs(n1 * sinI - n2 * sinT);
                node.entered_from_front_side = fromFront;
                node.medium_in_id = fromFront ? face.front_medium_id : face.back_medium_id;
                node.medium_out_id = fromFront ? face.back_medium_id : face.front_medium_id;
                node.front_medium_id = face.front_medium_id;
                node.back_medium_id = face.back_medium_id;
                node.front_material_id = face.front_material_id;
                node.back_material_id = face.back_material_id;
                node.transmission_semantic_complete = face.transmission_semantic_complete;
                node.snell_theta_i_rad = std::asin(std::min(1.0, sinI));
                node.snell_theta_t_rad = std::asin(std::min(1.0, sinT));
                node.snell_tir = tir;
                path.max_snell_residual = std::max(path.max_snell_residual,
                                                   std::max(node.snell_residual, directionResidual));
            }
        }
    }
    path.reflection_residual_m = path.max_reflection_residual;
    path.geometry_residual = std::max(path.max_reflection_residual, path.max_snell_residual);
    path.contains_transmission = SbrPathContainsTransmission(path);
}

bool PathSegmentsVisible(const GeometricPath& path, const SceneQuery& query, std::string* reason) {
    for (std::size_t i = 1; i < path.nodes.size(); ++i) {
        VisibilityQueryContext context;
        context.ignored_face_id = path.nodes[i - 1].face_id;
        context.ignored_face_id2 = path.nodes[i].face_id;
        if (!query.IsVisible(path.nodes[i - 1].point, path.nodes[i].point, context)) {
            if (reason) *reason = "refined segment is occluded";
            return false;
        }
    }
    return true;
}

std::string TopologyKey(const GeometricPath& path) {
    std::ostringstream key;
    key << (path.is_los ? 'L' : 'N') << ':';
    for (const PathNode& node : path.nodes) {
        key << static_cast<int>(node.interaction_type) << ',';
        if (node.interaction_type == InteractionType::Reflection ||
            node.interaction_type == InteractionType::Transmission) key << node.surface_patch_id;
        if (node.interaction_type == InteractionType::Transmission)
            key << '/' << node.medium_in_id << '/' << node.medium_out_id;
        if (node.interaction_type == InteractionType::Diffraction) key << node.wedge_id;
        key << ';';
    }
    return key.str();
}

double CandidateScore(const GeometricPath& path) {
    if (path.nodes.size() < 2) return 1.0e30;
    const PathNode& previous = path.nodes[path.nodes.size() - 2];
    const PathNode& rx = path.nodes.back();
    const Vec3 toRx = Subtract(rx.point, previous.point);
    const double miss = Length(Cross(toRx, Normalize(previous.direction)));
    return miss - 1.0e-9 * static_cast<double>(path.candidate_support_count);
}

} // namespace

std::vector<int> BuildSbrSurfacePatchIds(const Scene& scene) {
    const int count = static_cast<int>(scene.faces.size());
    DisjointSet sets(count);
    for (const Edge& edge : scene.edges) {
        if (!ValidFaceId(scene, edge.face_id0) || !ValidFaceId(scene, edge.face_id1)) continue;
        if (SamePatchSemantics(scene.faces[edge.face_id0], scene.faces[edge.face_id1]))
            sets.Join(edge.face_id0, edge.face_id1);
    }
    std::unordered_map<int, int> rootToPatch;
    std::vector<int> result(count, -1);
    int nextPatch = 0;
    for (int faceId = 0; faceId < count; ++faceId) {
        const int root = sets.Find(faceId);
        auto inserted = rootToPatch.emplace(root, nextPatch);
        if (inserted.second) ++nextPatch;
        result[faceId] = inserted.first->second;
    }
    return result;
}

bool RefineSbrPathGeometry(GeometricPath& path, const Scene& scene, const SceneQuery* query,
                           const MaterialDatabase* materialDb, double frequencyHz,
                           const std::vector<int>& faceToPatch, std::string* reason) {
    if (!path.valid || path.nodes.size() < 2) { if (reason) *reason = "invalid candidate"; return false; }
    int reflectionCount = 0, transmissionCount = 0, diffractionCount = 0;
    for (PathNode& node : path.nodes) {
        if (node.interaction_type == InteractionType::Reflection) ++reflectionCount;
        if (node.interaction_type == InteractionType::Transmission) ++transmissionCount;
        if (node.interaction_type == InteractionType::Diffraction) ++diffractionCount;
        if (ValidFaceId(scene, node.face_id) && node.face_id < static_cast<int>(faceToPatch.size()))
            node.surface_patch_id = faceToPatch[node.face_id];
    }
    bool refined = true;
    if (reflectionCount > 0 && transmissionCount == 0 && diffractionCount == 0)
        refined = RefineByImageMethod(path, scene, faceToPatch, reason);
    else if (transmissionCount > 0 && diffractionCount == 0)
        refined = RefineByLevenbergMarquardt(path, scene, materialDb, frequencyHz, faceToPatch, reason);
    else if (diffractionCount > 0) {
        path.refinement_method = "analytical_diffraction";
        path.refinement_iterations = 1;
    } else {
        path.refinement_method = "deterministic_los";
        path.refinement_iterations = 1;
    }
    if (!refined) return false;
    RecomputePathGeometry(path, scene, materialDb, frequencyHz);
    if (path.max_reflection_residual > kDirectionTolerance || path.max_snell_residual > kDirectionTolerance) {
        if (reason) *reason = "physical direction residual exceeds tolerance";
        return false;
    }
    if (query && !PathSegmentsVisible(path, *query, reason)) return false;
    path.geometry_refined = true;
    path.sampling_weight = 1.0;
    path.residual_reject_reason.clear();
    path.path_signature = SbrBuildPathSignature(path, 1.0e-6, 1.0e-7);
    return true;
}

SbrPathRefineStats RefineSbrCandidatesForEm(std::vector<GeometricPath>& paths, const Scene& scene,
                                             const SceneQuery& query, const MaterialDatabase* materialDb,
                                             double frequencyHz) {
    SbrPathRefineStats stats;
    stats.input_candidates = static_cast<long long>(paths.size());
    const std::vector<int> faceToPatch = BuildSbrSurfacePatchIds(scene);
    std::unordered_map<std::string, std::vector<std::size_t>> groups;
    for (std::size_t i = 0; i < paths.size(); ++i) {
        for (PathNode& node : paths[i].nodes)
            if (ValidFaceId(scene, node.face_id)) node.surface_patch_id = faceToPatch[node.face_id];
        groups[TopologyKey(paths[i])].push_back(i);
    }
    stats.topology_groups = static_cast<long long>(groups.size());
    std::vector<GeometricPath> refined;
    refined.reserve(groups.size());
    for (auto& entry : groups) {
        auto& candidates = entry.second;
        std::stable_sort(candidates.begin(), candidates.end(), [&](std::size_t a, std::size_t b) {
            return CandidateScore(paths[a]) < CandidateScore(paths[b]);
        });
        int support = 0;
        for (std::size_t index : candidates) support += std::max(1, paths[index].candidate_support_count);
        bool accepted = false;
        for (std::size_t index : candidates) {
            GeometricPath candidate = paths[index];
            std::string reason;
            if (RefineSbrPathGeometry(candidate, scene, &query, materialDb, frequencyHz, faceToPatch, &reason)) {
                candidate.candidate_support_count = support;
                refined.push_back(std::move(candidate));
                ++stats.refined_paths;
                accepted = true;
                break;
            }
            paths[index].residual_reject_reason = reason;
        }
        if (!accepted) ++stats.rejected_paths;
    }
    std::sort(refined.begin(), refined.end(), [](const GeometricPath& a, const GeometricPath& b) {
        if (a.nodes.size() != b.nodes.size()) return a.nodes.size() < b.nodes.size();
        if (a.total_length != b.total_length) return a.total_length < b.total_length;
        return a.path_signature < b.path_signature;
    });
    paths = std::move(refined);
    return stats;
}

} // namespace rt
