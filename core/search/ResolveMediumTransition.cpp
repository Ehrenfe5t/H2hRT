// 文件目标：
// - 实现模块4批次6的透射介质切换解析逻辑。
//
// 主要功能：
// - 根据面法向与当前介质状态确定 medium in/out；
// - 在批次6中优先保证显式结果与失败可诊断；
// - 为后续模块5 transmission 电磁求解提供稳定语义基础。

#include "ResolveMediumTransition.h"

#include <map>

namespace rt {

namespace {

int MaterialNameToId(const std::string& materialName)
{
    static std::map<std::string, int> dictionary;
    const auto found = dictionary.find(materialName);
    if (found != dictionary.end())
    {
        return found->second;
    }
    const int id = static_cast<int>(dictionary.size());
    dictionary[materialName] = id;
    return id;
}

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

} // namespace

/// <summary>
/// 解析透射交互下的介质切换信息。
/// </summary>
/// <param name="state">当前路径状态。</param>
/// <param name="hit">透射候选面元命中。</param>
/// <param name="context">搜索上下文。</param>
/// <returns>结构化介质切换结果。</returns>
MediumTransitionInfo ResolveMediumTransition(const PathState& state, const FaceHit& hit, const PathSearchContext& context)
{
    MediumTransitionInfo info;
    if (!hit.hit || hit.face_id < 0 || hit.face_id >= static_cast<int>(context.scene->faces.size()))
    {
        return info;
    }

    const Face& face = context.scene->faces[hit.face_id];
    if (!face.dual_side_material_resolved)
    {
        return info;
    }

    const bool frontSide = Dot(state.current_direction, face.normal) < 0.0;
    info.medium_in_id = state.current_medium_id;
    info.medium_out_id = frontSide ? MaterialNameToId(face.back_material_name) : MaterialNameToId(face.front_material_name);
    info.entered_from_front_side = frontSide;
    info.valid = info.medium_out_id >= 0;
    return info;
}

} // namespace rt
