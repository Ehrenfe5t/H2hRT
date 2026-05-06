// 文件目标：
// - 定义模块2在批次2阶段的统一 Scene 基础语义层对象。
//
// 主要功能：
// - 汇总 OBJ 导入的原始几何与面元语义；
// - 承载对象级材质绑定结果；
// - 在批次3中继续承载拓扑、诊断与加速结构结果；
// - 为后续模块4/5提供稳定的场景真源输入。

#pragma once

#include "Edge.h"
#include "Face.h"
#include "SceneMaterialBinding.h"
#include "SceneMeta.h"
#include "Wedge.h"

#include <string>
#include <cstdint>
#include <memory>
#include <vector>

namespace rt { class SceneQuery; }

namespace rt {

/// <summary>
/// 场景对象记录。
/// </summary>
struct SceneObjectRecord {
    int object_id = -1;
    std::string object_name;
    std::vector<int> face_ids;
};

/// <summary>
/// 场景诊断结果。
/// </summary>
struct SceneDiagnostics {
    std::vector<int> degenerate_faces;
    std::vector<int> non_manifold_edges;
    std::vector<int> duplicated_faces;
    std::vector<int> flipped_normal_faces;
    std::vector<std::string> objects_missing_material_mapping;
    std::vector<std::string> objects_matched_by_pattern;
    std::vector<std::string> objects_resolved_with_default_materials;
    std::vector<std::string> objects_with_partial_semantic_recovery;
    std::vector<std::string> unresolved_binding_objects;
    std::vector<int> faces_missing_dual_side_material;
    std::vector<int> transmission_faces_missing_semantics;
    std::vector<std::string> transmission_objects_missing_semantics;
    std::vector<std::string> warnings;
    bool passed = false;
};

/// <summary>
/// 面元高频查询记录。
/// </summary>
struct FaceQueryRecord {
    int face_id = -1;
    int object_id = -1;
    Vec3 normal;
    Point3 centroid;
    int front_material_id = -1;
    int back_material_id = -1;
    std::uint32_t propagation_flags = FacePropagationNone;
    int adjacent_edge_id0 = -1;
    int adjacent_edge_id1 = -1;
    int adjacent_edge_id2 = -1;
    LocalFrame local_frame;
    AABB bounds;
};

/// <summary>
/// 楔边高频查询记录。
/// </summary>
struct WedgeQueryRecord {
    int wedge_id = -1;
    int source_edge_id = -1;
    Point3 center_point;
    Point3 segment_start;
    Point3 segment_end;
    Vec3 direction;
    double length = 0.0;
    double wedge_angle_deg = 0.0;
    double dihedral_angle_deg = 0.0;
    int positive_face_id = -1;
    int negative_face_id = -1;
    int positive_material_id = -1;
    int negative_material_id = -1;
    std::uint32_t wedge_flags = WedgeFlagNone;
    AABB bounds;
};

/// <summary>
/// 面元 BVH 节点结构。
/// </summary>
struct FaceBVHNode {
    int node_id = -1;
    AABB bounds;
    int left_child = -1;
    int right_child = -1;
    int start_index = 0;
    int primitive_count = 0;
    int depth = 0;
    bool is_leaf = false;
};

/// <summary>
/// 面元 BVH 结构。
/// </summary>
struct FaceBVH {
    std::vector<FaceBVHNode> nodes;
    std::vector<int> primitive_face_ids;
    bool valid = false;
};

/// <summary>
/// 面元加速层。
/// </summary>
struct SceneFaceAcceleration {
    AABB scene_bounds;
    FaceBVH face_bvh;
    std::vector<FaceQueryRecord> face_query_records;
    bool valid = false;
    int bvh_node_count = 0;
    int leaf_node_count = 0;
};

/// <summary>
/// 楔边加速层。
/// </summary>
struct SceneWedgeAcceleration {
    std::vector<WedgeQueryRecord> wedge_query_records;
    bool valid = false;
    int wedge_count = 0;
    int diffractable_wedge_count = 0;
};

/// <summary>
/// 加速结构诊断结果。
/// </summary>
struct SceneAccelerationDiagnostics {
    bool build_succeeded = false;
    bool brute_force_validation_ran = false;
    bool brute_force_validation_passed = false;
    double face_bvh_build_time_ms = 0.0;
    double wedge_acceleration_build_time_ms = 0.0;
    int face_bvh_max_depth = 0;
    int face_bvh_average_leaf_faces = 0;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

/// <summary>
/// 场景统一加速结构容器。
/// </summary>
struct SceneAcceleration {
    SceneFaceAcceleration face_acceleration;
    SceneWedgeAcceleration wedge_acceleration;
    SceneAccelerationDiagnostics diagnostics;
};

/// <summary>
/// 模块2批次2阶段统一场景对象。
/// </summary>
struct Scene {
    SceneMeta meta;
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<SceneObjectRecord> objects;
    std::vector<Face> faces;
    std::vector<SceneMaterialBinding> material_bindings;
    std::vector<Edge> edges;
    std::vector<Wedge> wedges;
    SceneDiagnostics diagnostics;
    SceneAcceleration acceleration;
    std::shared_ptr<SceneQuery> query;
};

} // namespace rt
