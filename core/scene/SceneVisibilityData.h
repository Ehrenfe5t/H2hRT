// v8 Phase 1: 场景预计算可见性数据 — PVS + Edge Adjacency + Angular Grid + Room Graph
// 离线构建, 持久化到 SceneCache, 供 Stage 1-3 在线使用
#pragma once

#include "Face.h"  // for AABB, Face
#include <cstddef>
#include <string>
#include <vector>

namespace rt {

struct AABB;

/// Per-face Potentially Visible Set: face i 的半球可见面元集
/// Kim et al., "Visibility Precomputation for Accelerated Ray Tracing in Indoor Radio Propagation", IEEE AWPL 2020
struct FacePVS {
    std::vector<std::vector<int>> pvs_faces;  // pvs[i] = {face_j | face_j 从 face_i 半球可见}
    // v10 Iter0: 反向PVS — reverse_pvs[j] = {face_i | face_i 能看到 face_j}
    // 用于双向收缩: 正向扩张用 pvs_faces, 反向扩张用 reverse_pvs
    std::vector<std::vector<int>> reverse_pvs;
    size_t total_entries = 0;
    int hemisphere_sample_count = 200;         // 每面元的半球探测射线数
    bool valid = false;

    bool HasEntry(int faceIdx) const {
        return faceIdx >= 0 && faceIdx < static_cast<int>(pvs_faces.size()) && !pvs_faces[faceIdx].empty();
    }
    const std::vector<int>& GetVisibleFaces(int faceIdx) const {
        static const std::vector<int> empty;
        return (faceIdx >= 0 && faceIdx < static_cast<int>(pvs_faces.size())) ? pvs_faces[faceIdx] : empty;
    }
};

/// Edge-to-Edge Visibility Graph: 楔边间的互相可见性
/// Tiberi et al., "Efficient Path Finding with Multiple Diffraction Using Edge-to-Edge Visibility Graph", IEEE TAP 2021
struct EdgeAdjacency {
    std::vector<std::vector<int>> adjacent_wedges;  // adj[i] = {与 wedge_i 互相可见的楔边}
    size_t total_edges = 0;
    bool valid = false;

    bool HasEntry(int wedgeIdx) const {
        return wedgeIdx >= 0 && wedgeIdx < static_cast<int>(adjacent_wedges.size()) && !adjacent_wedges[wedgeIdx].empty();
    }
    const std::vector<int>& GetAdjacentWedges(int wedgeIdx) const {
        static const std::vector<int> empty;
        return (wedgeIdx >= 0 && wedgeIdx < static_cast<int>(adjacent_wedges.size())) ? adjacent_wedges[wedgeIdx] : empty;
    }
};

/// Angular Face Partition Grid: 4π球面等角度分区 → 该方向可见的面元集合
/// 用于 SBR 粗扫: 射线方向 → O(1)查表 → 候选面元 (~10-30)
struct AngularFaceGrid {
    std::vector<std::vector<int>> cells;  // cells[azi * nZenith + zen] = {face_ids}
    int n_azimuth = 64;
    int n_zenith = 32;
    bool valid = false;

    int CellCount() const { return n_azimuth * n_zenith; }
    int CellIndex(int azi, int zen) const { return azi * n_zenith + zen; }
    const std::vector<int>& GetFaces(int azi, int zen) const {
        static const std::vector<int> empty;
        int idx = CellIndex(azi, zen);
        return (idx >= 0 && idx < static_cast<int>(cells.size())) ? cells[idx] : empty;
    }
};

/// Room Topology Graph: 房间连通关系, 用于大型多房间场景的面元搜索剪枝
/// Yun & Iskander, "Ray Tracing for Radio Propagation Modeling", IEEE Access 2015
struct RoomGraph {
    struct Room {
        int room_id = -1;
        AABB bounds;
        std::vector<int> face_ids;  // 属于此房间的面元索引
    };
    struct RoomPortal {
        int room_a = -1, room_b = -1;
        std::vector<int> portal_face_ids;  // 门/窗/开口的面元
    };
    std::vector<Room> rooms;
    std::vector<RoomPortal> portals;
    std::vector<std::vector<int>> adjacency;  // adjacency[room_i] = 相邻房间
    bool valid = false;
};

/// 场景可见性预计算数据聚合 — 追加到 Scene 结构
struct SceneVisibilityData {
    FacePVS face_pvs;
    EdgeAdjacency edge_adjacency;
    AngularFaceGrid angular_grid;
    RoomGraph room_graph;

    bool valid = false;
    double build_time_seconds = 0.0;
    std::string build_timestamp;
};

} // namespace rt
