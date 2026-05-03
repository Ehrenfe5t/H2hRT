// 文件目标：
// - 实现模块2批次3的场景楔边构建器。
//
// 主要功能：
// - 从边集合筛选可构成楔边的对象；
// - 计算楔边长度、方向、角度与包围盒；
// - 按配置显式过滤非流形来源、共面边与角度异常来源。

#include "WedgeBuilder.h"

#include <cmath>

namespace rt {

namespace {

double ClampToUnit(double value)
{
    if (value < -1.0)
    {
        return -1.0;
    }
    if (value > 1.0)
    {
        return 1.0;
    }
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

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double Length(const Vec3& value)
{
    return std::sqrt(Dot(value, value));
}

Vec3 Normalize(const Vec3& value)
{
    const double length = Length(value);
    if (length <= 0.0)
    {
        return Vec3{};
    }

    Vec3 result;
    result.x = value.x / length;
    result.y = value.y / length;
    result.z = value.z / length;
    return result;
}

} // namespace

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

        if (edge.is_non_manifold && config.scene_preprocess.filter_non_manifold_wedge_sources)
        {
            continue;
        }

        if (edge.is_coplanar && config.scene_preprocess.skip_coplanar_edges_for_wedge)
        {
            continue;
        }

        const Face& positiveFace = scene.faces[edge.face_id0];
        const Face& negativeFace = scene.faces[edge.face_id1];
        if (!positiveFace.diffraction_candidate_enabled && !negativeFace.diffraction_candidate_enabled)
        {
            continue;
        }

        const double dihedral = edge.dihedral_angle_deg;
        const double wedgeAngle = 180.0 - dihedral;
        if (wedgeAngle < config.scene_preprocess.wedge_min_angle_deg ||
            wedgeAngle > config.scene_preprocess.wedge_max_angle_deg)
        {
            continue;
        }

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
        wedge.positive_material_name = positiveFace.back_material_name;
        wedge.negative_material_name = negativeFace.back_material_name;
        wedge.from_non_manifold_source = edge.is_non_manifold;
        wedge.diffractable = true;
        wedge.wedge_flags = WedgeFlagDiffractable;
        wedge.bounds = BuildBounds(wedge.segment_start, wedge.segment_end);

        if (edge.is_non_manifold)
        {
            wedge.wedge_flags |= WedgeFlagNonManifoldSource;
        }

        edge.supports_wedge = true;
        scene.wedges.push_back(wedge);
    }
}

} // namespace rt
