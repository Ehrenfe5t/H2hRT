// 文件目标：
// - 实现模块2批次3的楔边轻量加速记录构建器。
//
// 主要功能：
// - 从 Wedge 真源生成紧凑查询记录；
// - 对两侧主材料生成稳定整数索引；
// - 汇总楔边总数与可绕射楔边数量。

#include "WedgeAccelerationBuilder.h"

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

    scene.acceleration.wedge_acceleration.valid = !scene.acceleration.wedge_acceleration.wedge_query_records.empty();
}

} // namespace rt
