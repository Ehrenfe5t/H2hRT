// 文件目标：
// - 实现模块2批次3的面元 BVH 构建器。
//
// 主要功能：
// - 生成 FaceQueryRecord；
// - 基于有效面元递归构建 AABB-BVH；
// - 回写节点统计与场景总包围盒。

#include "FaceBVHBuilder.h"
#include "../../core/common/math/Vec3.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace rt {

namespace {

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
        // v7.3 A4: 叶节点内按面元面积降序排列 (大面更可能先命中→提前终止更早)
        std::sort(&primitiveFaceIds[begin], &primitiveFaceIds[end],
            [&faces](int a, int b) { return faces[a].area > faces[b].area; });
        ++leafNodeCount;
        return nodeIndex;
    }

    AABB centroidBounds;
    for (int i = begin; i < end; ++i) {
        const Point3 c = faces[primitiveFaceIds[i]].centroid;
        AABB pb; pb.min = c; pb.max = c; pb.valid = true;
        centroidBounds = MergeBounds(centroidBounds, pb);
    }

    // SAH 16-bin optimal split — replaces median split (v4 C4-A)
    const int NBINS = 16;
    const int nTris = end - begin;
    if (nTris <= leafSize * 2) {
        // Small node: use median
        const double dx = centroidBounds.max.x - centroidBounds.min.x;
        const double dy = centroidBounds.max.y - centroidBounds.min.y;
        const double dz = centroidBounds.max.z - centroidBounds.min.z;
        const int axis = (dx >= dy && dx >= dz) ? 0 : ((dy >= dz) ? 1 : 2);
        std::sort(primitiveFaceIds.begin() + begin, primitiveFaceIds.begin() + end,
            [&](int l, int r) {
                const Point3& a = faces[l].centroid; const Point3& b = faces[r].centroid;
                return (axis == 0) ? a.x < b.x : ((axis == 1) ? a.y < b.y : a.z < b.z);
            });
        int middle = begin + nTris / 2;
        const int leftChild = BuildBVHNodeRecursive(bvh, primitiveFaceIds, faces, begin, middle, depth + 1, leafSize, leafNodeCount, maxDepth);
        const int rightChild = BuildBVHNodeRecursive(bvh, primitiveFaceIds, faces, middle, end, depth + 1, leafSize, leafNodeCount, maxDepth);
        bvh.nodes[nodeIndex].left_child = leftChild;
        bvh.nodes[nodeIndex].right_child = rightChild;
        return nodeIndex;
    }

    // SAH: try 3 axes, pick minimal cost
    int bestAxis = 0; int bestSplit = begin + nTris / 2; double bestCost = 1e30;
    double parentArea = (node.bounds.max.x - node.bounds.min.x) * (node.bounds.max.y - node.bounds.min.y) * (node.bounds.max.z - node.bounds.min.z);
    if (parentArea <= 0.0) parentArea = 1.0;

    for (int ax = 0; ax < 3; ++ax) {
        // Build 16 bins along this axis
        double cMin = (ax == 0) ? centroidBounds.min.x : ((ax == 1) ? centroidBounds.min.y : centroidBounds.min.z);
        double cMax = (ax == 0) ? centroidBounds.max.x : ((ax == 1) ? centroidBounds.max.y : centroidBounds.max.z);
        if (cMax - cMin < 1e-12) continue;

        int binCounts[NBINS] = {0};
        AABB binBounds[NBINS];
        for (int i = begin; i < end; ++i) {
            double cv = (ax == 0) ? faces[primitiveFaceIds[i]].centroid.x : ((ax == 1) ? faces[primitiveFaceIds[i]].centroid.y : faces[primitiveFaceIds[i]].centroid.z);
            int b = std::min(NBINS - 1, static_cast<int>((cv - cMin) / (cMax - cMin) * NBINS));
            binCounts[b]++;
            binBounds[b] = MergeBounds(binBounds[b], faces[primitiveFaceIds[i]].bounds);
        }

        // Evaluate split at each bin boundary (split after bin k)
        int leftCount = 0; AABB leftBounds;
        for (int k = 0; k < NBINS - 1; ++k) {
            leftCount += binCounts[k];
            leftBounds = MergeBounds(leftBounds, binBounds[k]);
            int rightCount = nTris - leftCount;
            if (leftCount == 0 || rightCount == 0) continue;
            // Right bounds: merge remaining bins
            AABB rightBounds;
            for (int rk = k + 1; rk < NBINS; ++rk) rightBounds = MergeBounds(rightBounds, binBounds[rk]);
            double leftArea = (leftBounds.max.x - leftBounds.min.x) * (leftBounds.max.y - leftBounds.min.y) * (leftBounds.max.z - leftBounds.min.z);
            double rightArea = (rightBounds.max.x - rightBounds.min.x) * (rightBounds.max.y - rightBounds.min.y) * (rightBounds.max.z - rightBounds.min.z);
            double cost = (leftArea * leftCount + rightArea * rightCount) / parentArea;
            if (cost < bestCost) { bestCost = cost; bestAxis = ax; bestSplit = begin + leftCount; }
        }
    }

    // Sort along best axis and split
    const int axis = bestAxis;
    std::sort(primitiveFaceIds.begin() + begin, primitiveFaceIds.begin() + end,
        [&](int l, int r) {
            const Point3& a = faces[l].centroid; const Point3& b = faces[r].centroid;
            return (axis == 0) ? a.x < b.x : ((axis == 1) ? a.y < b.y : a.z < b.z);
        });
    const int middle = std::max(begin + 1, std::min(end - 1, bestSplit));
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
