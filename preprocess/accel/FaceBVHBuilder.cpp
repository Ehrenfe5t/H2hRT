// 文件目标：
// - 实现模块2批次3的面元 BVH 构建器。
//
// 主要功能：
// - 生成 FaceQueryRecord；
// - 基于有效面元递归构建 AABB-BVH；
// - 回写节点统计与场景总包围盒。

#include "FaceBVHBuilder.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace rt {

namespace {

Vec3 MakeVec3(double x, double y, double z)
{
    Vec3 value;
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

Vec3 Subtract(const Vec3& a, const Vec3& b)
{
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return MakeVec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
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
    return MakeVec3(value.x / length, value.y / length, value.z / length);
}

Point3 Centroid(const Point3& a, const Point3& b, const Point3& c)
{
    return MakeVec3((a.x + b.x + c.x) / 3.0, (a.y + b.y + c.y) / 3.0, (a.z + b.z + c.z) / 3.0);
}

AABB BuildFaceBounds(const Point3& a, const Point3& b, const Point3& c)
{
    AABB bounds;
    bounds.min.x = std::min(a.x, std::min(b.x, c.x));
    bounds.min.y = std::min(a.y, std::min(b.y, c.y));
    bounds.min.z = std::min(a.z, std::min(b.z, c.z));
    bounds.max.x = std::max(a.x, std::max(b.x, c.x));
    bounds.max.y = std::max(a.y, std::max(b.y, c.y));
    bounds.max.z = std::max(a.z, std::max(b.z, c.z));
    bounds.valid = true;
    return bounds;
}

AABB MergeBounds(const AABB& a, const AABB& b)
{
    if (!a.valid)
    {
        return b;
    }
    if (!b.valid)
    {
        return a;
    }

    AABB result;
    result.min.x = std::min(a.min.x, b.min.x);
    result.min.y = std::min(a.min.y, b.min.y);
    result.min.z = std::min(a.min.z, b.min.z);
    result.max.x = std::max(a.max.x, b.max.x);
    result.max.y = std::max(a.max.y, b.max.y);
    result.max.z = std::max(a.max.z, b.max.z);
    result.valid = true;
    return result;
}

LocalFrame BuildLocalFrame(const Vec3& normal)
{
    LocalFrame frame;
    frame.normal = Normalize(normal);
    const Vec3 guide = (std::fabs(frame.normal.z) < 0.9) ? MakeVec3(0.0, 0.0, 1.0) : MakeVec3(0.0, 1.0, 0.0);
    frame.tangent = Normalize(Cross(guide, frame.normal));
    frame.bitangent = Normalize(Cross(frame.normal, frame.tangent));
    frame.valid = Length(frame.tangent) > 0.0 && Length(frame.bitangent) > 0.0 && Length(frame.normal) > 0.0;
    return frame;
}

int MaterialNameToId(const std::string& materialName, std::map<std::string, int>& dictionary)
{
    if (materialName.empty())
    {
        return -1;
    }

    const auto found = dictionary.find(materialName);
    if (found != dictionary.end())
    {
        return found->second;
    }

    const int nextId = static_cast<int>(dictionary.size());
    dictionary[materialName] = nextId;
    return nextId;
}

int BuildBVHNodeRecursive(
    FaceBVH& bvh,
    std::vector<int>& primitiveFaceIds,
    const std::vector<Face>& faces,
    int begin,
    int end,
    int depth,
    int leafSize,
    int& leafNodeCount,
    int& maxDepth)
{
    FaceBVHNode node;
    node.node_id = static_cast<int>(bvh.nodes.size());
    node.start_index = begin;
    node.primitive_count = end - begin;
    node.depth = depth;
    node.is_leaf = (end - begin) <= leafSize;

    AABB bounds;
    for (int i = begin; i < end; ++i)
    {
        const int faceId = primitiveFaceIds[i];
        bounds = MergeBounds(bounds, faces[faceId].bounds);
    }
    node.bounds = bounds;

    const int nodeIndex = static_cast<int>(bvh.nodes.size());
    bvh.nodes.push_back(node);
    if (depth > maxDepth)
    {
        maxDepth = depth;
    }

    if (node.is_leaf)
    {
        ++leafNodeCount;
        return nodeIndex;
    }

    AABB centroidBounds;
    for (int i = begin; i < end; ++i)
    {
        const Point3 centroid = faces[primitiveFaceIds[i]].centroid;
        AABB pointBounds;
        pointBounds.min = centroid;
        pointBounds.max = centroid;
        pointBounds.valid = true;
        centroidBounds = MergeBounds(centroidBounds, pointBounds);
    }

    const double dx = centroidBounds.max.x - centroidBounds.min.x;
    const double dy = centroidBounds.max.y - centroidBounds.min.y;
    const double dz = centroidBounds.max.z - centroidBounds.min.z;
    const int axis = (dx >= dy && dx >= dz) ? 0 : ((dy >= dz) ? 1 : 2);

    std::sort(primitiveFaceIds.begin() + begin, primitiveFaceIds.begin() + end,
        [&](int lhs, int rhs)
        {
            const Point3& a = faces[lhs].centroid;
            const Point3& b = faces[rhs].centroid;
            if (axis == 0)
            {
                return a.x < b.x;
            }
            if (axis == 1)
            {
                return a.y < b.y;
            }
            return a.z < b.z;
        });

    const int middle = begin + (end - begin) / 2;
    const int leftChild = BuildBVHNodeRecursive(bvh, primitiveFaceIds, faces, begin, middle, depth + 1, leafSize, leafNodeCount, maxDepth);
    const int rightChild = BuildBVHNodeRecursive(bvh, primitiveFaceIds, faces, middle, end, depth + 1, leafSize, leafNodeCount, maxDepth);

    bvh.nodes[nodeIndex].left_child = leftChild;
    bvh.nodes[nodeIndex].right_child = rightChild;
    return nodeIndex;
}

} // namespace

/// <summary>
/// 构建面元 BVH 及其查询记录。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待读取面元并写回面元加速结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildFaceBVHAcceleration(const AppConfig& config, Scene& scene)
{
    scene.acceleration.face_acceleration = SceneFaceAcceleration{};

    std::map<std::string, int> materialDictionary;
    std::vector<int> validFaceIds;
    for (Face& face : scene.faces)
    {
        if (face.vertex_index0 < 0 || face.vertex_index1 < 0 || face.vertex_index2 < 0 ||
            face.vertex_index0 >= static_cast<int>(scene.vertices.size()) ||
            face.vertex_index1 >= static_cast<int>(scene.vertices.size()) ||
            face.vertex_index2 >= static_cast<int>(scene.vertices.size()))
        {
            face.degenerate = true;
            continue;
        }

        const Point3& p0 = scene.vertices[face.vertex_index0];
        const Point3& p1 = scene.vertices[face.vertex_index1];
        const Point3& p2 = scene.vertices[face.vertex_index2];
        const Vec3 cross = Cross(Subtract(p1, p0), Subtract(p2, p0));
        face.area = 0.5 * Length(cross);
        face.degenerate = face.area <= config.numeric_tolerance.eps_length;
        face.centroid = Centroid(p0, p1, p2);
        face.bounds = BuildFaceBounds(p0, p1, p2);
        face.local_frame = BuildLocalFrame(face.normal);
        if (face.degenerate)
        {
            continue;
        }

        FaceQueryRecord record;
        record.face_id = face.face_id;
        record.object_id = face.object_id;
        record.normal = face.normal;
        record.centroid = face.centroid;
        record.front_material_id = MaterialNameToId(face.front_material_name, materialDictionary);
        record.back_material_id = MaterialNameToId(face.back_material_name, materialDictionary);
        record.propagation_flags = face.propagation_flags;
        record.adjacent_edge_id0 = face.adjacent_edge_id0;
        record.adjacent_edge_id1 = face.adjacent_edge_id1;
        record.adjacent_edge_id2 = face.adjacent_edge_id2;
        record.local_frame = face.local_frame;
        record.bounds = face.bounds;
        scene.acceleration.face_acceleration.face_query_records.push_back(record);
        validFaceIds.push_back(face.face_id);
        scene.acceleration.face_acceleration.scene_bounds = MergeBounds(scene.acceleration.face_acceleration.scene_bounds, face.bounds);
    }

    scene.acceleration.face_acceleration.valid = !validFaceIds.empty();
    if (!scene.acceleration.face_acceleration.valid)
    {
        return;
    }

    scene.acceleration.face_acceleration.face_bvh.primitive_face_ids = validFaceIds;
    int leafNodeCount = 0;
    int maxDepth = 0;
    BuildBVHNodeRecursive(
        scene.acceleration.face_acceleration.face_bvh,
        scene.acceleration.face_acceleration.face_bvh.primitive_face_ids,
        scene.faces,
        0,
        static_cast<int>(scene.acceleration.face_acceleration.face_bvh.primitive_face_ids.size()),
        0,
        config.scene_preprocess.bvh_leaf_size,
        leafNodeCount,
        maxDepth);
    scene.acceleration.face_acceleration.face_bvh.valid = true;
    scene.acceleration.face_acceleration.bvh_node_count = static_cast<int>(scene.acceleration.face_acceleration.face_bvh.nodes.size());
    scene.acceleration.face_acceleration.leaf_node_count = leafNodeCount;
    scene.acceleration.diagnostics.face_bvh_max_depth = maxDepth;

    if (leafNodeCount > 0)
    {
        const int primitiveCount = static_cast<int>(scene.acceleration.face_acceleration.face_bvh.primitive_face_ids.size());
        scene.acceleration.diagnostics.face_bvh_average_leaf_faces = primitiveCount / leafNodeCount;
    }
}

} // namespace rt
