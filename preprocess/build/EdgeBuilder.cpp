// 文件目标：
// - 实现模块2批次3的场景边构建器。
//
// 主要功能：
// - 从三角面恢复唯一拓扑边；
// - 计算边长度、方向、二面角与边级分类；
// - 回填面元到边的邻接索引，供后续 wedge 与 query 使用。

#include "EdgeBuilder.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>
#include <vector>

namespace rt {

namespace {

struct EdgeBuildAccumulator {
    int vertex0 = -1;
    int vertex1 = -1;
    std::vector<int> face_ids;
};

Vec3 Subtract(const Vec3& a, const Vec3& b)
{
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

Vec3 Add(const Vec3& a, const Vec3& b)
{
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

Vec3 Scale(const Vec3& value, double factor)
{
    Vec3 result;
    result.x = value.x * factor;
    result.y = value.y * factor;
    result.z = value.z * factor;
    return result;
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

    return Scale(value, 1.0 / length);
}

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

std::pair<int, int> MakeEdgeKey(int a, int b)
{
    return (a < b) ? std::make_pair(a, b) : std::make_pair(b, a);
}

void AssignAdjacentEdgeToFace(Face& face, int edgeId)
{
    if (face.adjacent_edge_id0 < 0)
    {
        face.adjacent_edge_id0 = edgeId;
        return;
    }

    if (face.adjacent_edge_id1 < 0)
    {
        face.adjacent_edge_id1 = edgeId;
        return;
    }

    face.adjacent_edge_id2 = edgeId;
}

} // namespace

/// <summary>
/// 根据 Scene 面元集合构建唯一拓扑边并回填邻接。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待写入边构建结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildSceneEdges(const AppConfig& config, Scene& scene)
{
    static_cast<void>(config);

    scene.edges.clear();
    for (Face& face : scene.faces)
    {
        face.adjacent_edge_id0 = -1;
        face.adjacent_edge_id1 = -1;
        face.adjacent_edge_id2 = -1;
    }

    std::map<std::pair<int, int>, EdgeBuildAccumulator> edgeMap;
    for (const Face& face : scene.faces)
    {
        const int vertexIndices[3] = { face.vertex_index0, face.vertex_index1, face.vertex_index2 };
        for (int i = 0; i < 3; ++i)
        {
            const int a = vertexIndices[i];
            const int b = vertexIndices[(i + 1) % 3];
            const std::pair<int, int> key = MakeEdgeKey(a, b);
            EdgeBuildAccumulator& accumulator = edgeMap[key];
            accumulator.vertex0 = key.first;
            accumulator.vertex1 = key.second;
            accumulator.face_ids.push_back(face.face_id);
        }
    }

    for (const auto& item : edgeMap)
    {
        const EdgeBuildAccumulator& accumulator = item.second;
        Edge edge;
        edge.edge_id = static_cast<int>(scene.edges.size());
        edge.vertex_index0 = accumulator.vertex0;
        edge.vertex_index1 = accumulator.vertex1;
        edge.face_id0 = accumulator.face_ids.empty() ? -1 : accumulator.face_ids[0];
        edge.face_id1 = accumulator.face_ids.size() >= 2 ? accumulator.face_ids[1] : -1;
        edge.is_boundary = accumulator.face_ids.size() == 1;
        edge.is_non_manifold = accumulator.face_ids.size() > 2;

        if (edge.vertex_index0 >= 0 && edge.vertex_index0 < static_cast<int>(scene.vertices.size()) &&
            edge.vertex_index1 >= 0 && edge.vertex_index1 < static_cast<int>(scene.vertices.size()))
        {
            edge.start = scene.vertices[edge.vertex_index0];
            edge.end = scene.vertices[edge.vertex_index1];
            const Vec3 delta = Subtract(edge.end, edge.start);
            edge.length = Length(delta);
            edge.direction = Normalize(delta);
            edge.midpoint = Scale(Add(edge.start, edge.end), 0.5);
        }

        if (edge.face_id0 >= 0 && edge.face_id0 < static_cast<int>(scene.faces.size()) &&
            edge.face_id1 >= 0 && edge.face_id1 < static_cast<int>(scene.faces.size()))
        {
            const Vec3 normal0 = scene.faces[edge.face_id0].normal;
            const Vec3 normal1 = scene.faces[edge.face_id1].normal;
            const double cosine = ClampToUnit(Dot(Normalize(normal0), Normalize(normal1)));
            edge.dihedral_angle_deg = std::acos(cosine) * 180.0 / 3.14159265358979323846;
            edge.is_coplanar = std::fabs(edge.dihedral_angle_deg) <= config.numeric_tolerance.eps_angle * 180.0;
        }

        scene.edges.push_back(edge);

        if (edge.face_id0 >= 0 && edge.face_id0 < static_cast<int>(scene.faces.size()))
        {
            AssignAdjacentEdgeToFace(scene.faces[edge.face_id0], edge.edge_id);
        }
        if (edge.face_id1 >= 0 && edge.face_id1 < static_cast<int>(scene.faces.size()))
        {
            AssignAdjacentEdgeToFace(scene.faces[edge.face_id1], edge.edge_id);
        }
    }
}

} // namespace rt
