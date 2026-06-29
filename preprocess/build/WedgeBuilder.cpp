// 文件目标：
// - 实现模块2批次3的场景楔边构建器。
//
// 主要功能：
// - 从边集合筛选可构成楔边的对象；
// - 计算楔边长度、方向、角度与包围盒；
// - 按配置显式过滤非流形来源、共面边与角度异常来源。

#include "WedgeBuilder.h"
#include "../../core/common/math/Vec3.h"

#include <cmath>

namespace rt {

namespace {

double ClampToUnit(double value)
{
    if (value < -1.0) return -1.0;
    if (value > 1.0) return 1.0;
    return value;
}

AABB BuildBounds(const Point3& a, const Point3& b)
{
    AABB bounds;
    bounds.min.x = (a.x < b.x) ? a.x : b.x;
    bounds.min.y = (a.y < b.y) ? a.y : b.y;
    bounds.min.z = (a.z < b.z) ? a.z : b.z;
    bounds.max.x = (a.x > b.x) ? a.x : b.x;
    bounds.max.y = (a.y > b.y) ? a.y : b.y;
    bounds.max.z = (a.z > b.z) ? a.z : b.z;
    bounds.valid = true;
    return bounds;
}

int DirectedEdgeSignInFace(const Face& face, int vertex0, int vertex1)
{
    const int vertices[3] = {face.vertex_index0, face.vertex_index1, face.vertex_index2};
    for (int i = 0; i < 3; ++i)
    {
        const int a = vertices[i];
        const int b = vertices[(i + 1) % 3];
        if (a == vertex0 && b == vertex1) return 1;
        if (a == vertex1 && b == vertex0) return -1;
    }
    return 0;
}

Vec3 RawGeometricNormal(const Scene& scene, const Face& face)
{
    if (face.vertex_index0 < 0 || face.vertex_index1 < 0 || face.vertex_index2 < 0 ||
        face.vertex_index0 >= static_cast<int>(scene.vertices.size()) ||
        face.vertex_index1 >= static_cast<int>(scene.vertices.size()) ||
        face.vertex_index2 >= static_cast<int>(scene.vertices.size())) return Vec3{};
    return Normalize(Cross(
        Subtract(scene.vertices[face.vertex_index1], scene.vertices[face.vertex_index0]),
        Subtract(scene.vertices[face.vertex_index2], scene.vertices[face.vertex_index0])));
}

} // namespace

WedgeConvexity ClassifySharedEdgeConvexity(const Scene& scene,
                                           const Edge& edge,
                                           double* signedNormalTurnDeg)
{
    if (signedNormalTurnDeg) *signedNormalTurnDeg = 0.0;
    if (edge.face_id0 < 0 || edge.face_id1 < 0 || edge.is_non_manifold || edge.is_coplanar ||
        edge.face_id0 >= static_cast<int>(scene.faces.size()) ||
        edge.face_id1 >= static_cast<int>(scene.faces.size()))
        return WedgeConvexity::Unknown;

    const Face& face0 = scene.faces[edge.face_id0];
    const Face& face1 = scene.faces[edge.face_id1];
    int edgeSign = DirectedEdgeSignInFace(face0, edge.vertex_index0, edge.vertex_index1);
    if (edgeSign == 0) return WedgeConvexity::Unknown;

    // Imported normals orient propagation geometry but do not necessarily
    // reorder the OBJ triangle indices. Correct the traversal when required.
    const Vec3 raw0 = RawGeometricNormal(scene, face0);
    if (Length(raw0) <= 0.5 || Length(face0.normal) <= 0.5 || Length(face1.normal) <= 0.5)
        return WedgeConvexity::Unknown;
    if (Dot(raw0, face0.normal) < 0.0) edgeSign = -edgeSign;

    const Vec3 edgeDirection = Scale(Normalize(edge.direction), static_cast<double>(edgeSign));
    const Vec3 normal0 = Normalize(face0.normal);
    const Vec3 normal1 = Normalize(face1.normal);
    const double sine = Dot(edgeDirection, Cross(normal0, normal1));
    const double cosine = ClampToUnit(Dot(normal0, normal1));
    const double signedTurn = std::atan2(sine, cosine) * 180.0 / 3.14159265358979323846;
    if (signedNormalTurnDeg) *signedNormalTurnDeg = signedTurn;

    const double orientationToleranceDeg = 1.0e-7;
    if (signedTurn > orientationToleranceDeg) return WedgeConvexity::Convex;
    if (signedTurn < -orientationToleranceDeg) return WedgeConvexity::Concave;
    return WedgeConvexity::Unknown;
}

/// <summary>
/// 根据 Edge 集合构建 Scene 楔边集合。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入楔边构建结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneWedges(const AppConfig& config, Scene& scene)
{
    scene.wedges.clear();

    if (!config.scene_preprocess.enable_wedge_build)
    {
        return;
    }

    for (Edge& edge : scene.edges)
    {
        edge.supports_wedge = false;

        if (edge.face_id0 < 0 || edge.face_id1 < 0)
        {
            continue;
        }

        if (edge.is_non_manifold) { continue; }  // v6: always filter
        if (edge.is_coplanar) { continue; }       // v6: always filter

        const Face& positiveFace = scene.faces[edge.face_id0];
        const Face& negativeFace = scene.faces[edge.face_id1];
        if (!positiveFace.diffraction_candidate_enabled && !negativeFace.diffraction_candidate_enabled)
        {
            continue;
        }

        const double dihedral = edge.dihedral_angle_deg;
        if (dihedral < 3.0 || dihedral > 177.0)
        {
            continue;
        }

        double signedNormalTurnDeg = 0.0;
        const WedgeConvexity convexity =
            ClassifySharedEdgeConvexity(scene, edge, &signedNormalTurnDeg);
        if (convexity == WedgeConvexity::Unknown ||
            (config.scene_preprocess.convex_wedges_only && convexity != WedgeConvexity::Convex))
        {
            continue;
        }

        // Convex solid: exterior opening = 180 deg + normal turn.
        // Concave solid: exterior opening = normal turn.
        const double wedgeAngle = convexity == WedgeConvexity::Convex
            ? 180.0 + std::fabs(signedNormalTurnDeg)
            : std::fabs(signedNormalTurnDeg);

        Wedge wedge;
        wedge.wedge_id = static_cast<int>(scene.wedges.size());
        wedge.source_edge_id = edge.edge_id;
        wedge.positive_face_id = edge.face_id0;
        wedge.negative_face_id = edge.face_id1;
        wedge.segment_start = edge.start;
        wedge.segment_end = edge.end;
        wedge.center_point = edge.midpoint;
        wedge.direction = Normalize(edge.direction);
        wedge.length = edge.length;
        wedge.dihedral_angle_deg = dihedral;
        wedge.wedge_angle_deg = wedgeAngle;
        // UTD uses the radio material of each exposed wedge face. Prefer the
        // explicit surface material and retain the solid-side material as a
        // compatibility fallback for older scene maps.
        wedge.positive_material_name = !positiveFace.surface_material_name.empty()
            ? positiveFace.surface_material_name : positiveFace.back_material_name;
        wedge.negative_material_name = !negativeFace.surface_material_name.empty()
            ? negativeFace.surface_material_name : negativeFace.back_material_name;
        wedge.from_non_manifold_source = edge.is_non_manifold;
        wedge.diffractable = true;
        wedge.wedge_flags = WedgeFlagDiffractable;
        wedge.bounds = BuildBounds(wedge.segment_start, wedge.segment_end);

        // v9 D-6: 凸性 + UTD有效性
        wedge.zero_face_id = edge.face_id0; // 默认positive_face为UTD参考面
        wedge.convexity = convexity;
        // K&P exterior wedge angle is n*pi. wedge_angle_deg already stores
        // that exterior angle, so a 90-degree solid wedge gives n=1.5.
        double n = wedgeAngle / 180.0;
        wedge.valid_for_utd = (!edge.is_non_manifold) && (n >= 0.5 && n <= 2.0);

        if (edge.is_non_manifold)
        {
            wedge.wedge_flags |= WedgeFlagNonManifoldSource;
        }

        edge.supports_wedge = true;
        scene.wedges.push_back(wedge);
    }
}

} // namespace rt
