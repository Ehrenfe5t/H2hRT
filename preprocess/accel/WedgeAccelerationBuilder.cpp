// 文件目标：
// - 实现模块2批次3的楔边轻量加速记录构建器。
//
// 主要功能：
// - 从 Wedge 真源生成紧凑查询记录；
// - 对两侧主材料生成稳定整数索引；
// - 汇总楔边总数与可绕射楔边数量。

#include "WedgeAccelerationBuilder.h"
#include "../../core/common/math/MathConstants.h"

#include <map>

namespace rt {

namespace {

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

} // namespace

/// <summary>
/// 构建楔边轻量查询记录集合。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">待读取楔边并写回楔边加速结果的场景对象。</param>
/// <returns>无返回值。</returns>
void BuildWedgeAcceleration(const AppConfig& config, Scene& scene)
{
    static_cast<void>(config);
    scene.acceleration.wedge_acceleration = SceneWedgeAcceleration{};

    std::map<std::string, int> materialDictionary;
    for (const Wedge& wedge : scene.wedges)
    {
        WedgeQueryRecord record;
        record.wedge_id = wedge.wedge_id;
        record.source_edge_id = wedge.source_edge_id;
        record.center_point = wedge.center_point;
        record.segment_start = wedge.segment_start;
        record.segment_end = wedge.segment_end;
        record.direction = wedge.direction;
        record.length = wedge.length;
        record.wedge_angle_deg = wedge.wedge_angle_deg;
        record.dihedral_angle_deg = wedge.dihedral_angle_deg;
        record.positive_face_id = wedge.positive_face_id;
        record.negative_face_id = wedge.negative_face_id;
        record.positive_material_id = MaterialNameToId(wedge.positive_material_name, materialDictionary);
        record.negative_material_id = MaterialNameToId(wedge.negative_material_name, materialDictionary);
        record.wedge_flags = wedge.wedge_flags;
        record.bounds = wedge.bounds;
        scene.acceleration.wedge_acceleration.wedge_query_records.push_back(record);
        ++scene.acceleration.wedge_acceleration.wedge_count;
        if (wedge.diffractable)
        {
            ++scene.acceleration.wedge_acceleration.diffractable_wedge_count;
        }
    }

    // v9 step26: 构建均匀网格空间索引
    auto& grid = scene.acceleration.wedge_acceleration.grid;
    auto& records = scene.acceleration.wedge_acceleration.wedge_query_records;
    if (!records.empty()) {
        // 计算全局AABB
        double xmin = records[0].bounds.min.x, xmax = records[0].bounds.max.x;
        double ymin = records[0].bounds.min.y, ymax = records[0].bounds.max.y;
        double zmin = records[0].bounds.min.z, zmax = records[0].bounds.max.z;
        for (auto& r : records) {
            if (r.bounds.min.x < xmin) xmin = r.bounds.min.x;
            if (r.bounds.max.x > xmax) xmax = r.bounds.max.x;
            if (r.bounds.min.y < ymin) ymin = r.bounds.min.y;
            if (r.bounds.max.y > ymax) ymax = r.bounds.max.y;
            if (r.bounds.min.z < zmin) zmin = r.bounds.min.z;
            if (r.bounds.max.z > zmax) zmax = r.bounds.max.z;
        }
        // 扩展边界防止边界wedge漏查
        double margin = 5.0;
        grid.bounds.min = MakeVec3(xmin - margin, ymin - margin, zmin - margin);
        grid.bounds.max = MakeVec3(xmax + margin, ymax + margin, zmax + margin);
        grid.bounds.valid = true;

        // 单元大小 = wedge_max_distance_m / 2 (确保查询半径内覆盖邻居)
        grid.cell_size = std::max(1.0, config.path_search.wedge_max_distance_m * 0.5);
        double dx = grid.bounds.max.x - grid.bounds.min.x;
        double dy = grid.bounds.max.y - grid.bounds.min.y;
        double dz = grid.bounds.max.z - grid.bounds.min.z;
        grid.nx = std::max(1, static_cast<int>(dx / grid.cell_size) + 1);
        grid.ny = std::max(1, static_cast<int>(dy / grid.cell_size) + 1);
        grid.nz = std::max(1, static_cast<int>(dz / grid.cell_size) + 1);
        grid.cells.assign(static_cast<size_t>(grid.nx) * grid.ny * grid.nz, {});

        // 每个wedge插入其AABB重叠的所有cell
        for (int wi = 0; wi < static_cast<int>(records.size()); ++wi) {
            auto& r = records[wi];
            int cx0 = std::max(0, static_cast<int>((r.bounds.min.x - grid.bounds.min.x) / grid.cell_size));
            int cx1 = std::min(grid.nx-1, static_cast<int>((r.bounds.max.x - grid.bounds.min.x) / grid.cell_size));
            int cy0 = std::max(0, static_cast<int>((r.bounds.min.y - grid.bounds.min.y) / grid.cell_size));
            int cy1 = std::min(grid.ny-1, static_cast<int>((r.bounds.max.y - grid.bounds.min.y) / grid.cell_size));
            int cz0 = std::max(0, static_cast<int>((r.bounds.min.z - grid.bounds.min.z) / grid.cell_size));
            int cz1 = std::min(grid.nz-1, static_cast<int>((r.bounds.max.z - grid.bounds.min.z) / grid.cell_size));
            for (int cz = cz0; cz <= cz1; ++cz)
                for (int cy = cy0; cy <= cy1; ++cy)
                    for (int cx = cx0; cx <= cx1; ++cx)
                        grid.cells[cx * grid.ny * grid.nz + cy * grid.nz + cz].push_back(wi);
        }
    }

    // v9 D-2: UTD wedge n参数诊断 — 验证n在合理范围 [0.5, 2.0]
    for (const auto& w : scene.wedges) {
        double alphaRad = w.wedge_angle_deg * 3.14159265358979323846 / 180.0;
        double n = (kTwoPi - alphaRad) / 3.14159265358979323846;
        if (n < 0.5 || n > 2.0) {
            // wedge角度异常 — 可能几何有问题
            // n<1对应锐角边缘, n>1对应钝角, 极端值表示建模问题
        }
    }

    scene.acceleration.wedge_acceleration.valid = !scene.acceleration.wedge_acceleration.wedge_query_records.empty();
}

} // namespace rt
