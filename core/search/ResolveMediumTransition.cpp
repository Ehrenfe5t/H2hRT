// 文件目标：
// - 实现模块4批次6的透射介质切换解析逻辑。
//
// 主要功能：
// - 根据面法向与当前介质状态确定 medium in/out；
// - 在批次6中优先保证显式结果与失败可诊断；
// - 为后续模块5 transmission 电磁求解提供稳定语义基础。

#include "ResolveMediumTransition.h"

namespace rt {

namespace {

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
    if (!face.dual_side_material_resolved || !face.transmission_semantic_complete)
    {
        return info;
    }

    const bool frontSide = Dot(state.current_direction, face.normal) < 0.0;
    info.face_id = hit.face_id;
    info.object_id = hit.object_id;
    info.front_medium_id = face.front_medium_id;
    info.back_medium_id = face.back_medium_id;
    info.front_material_id = face.front_material_id;
    info.back_material_id = face.back_material_id;
    info.dual_side_semantic_complete = face.transmission_semantic_complete;
    info.medium_in_id = state.current_medium_id;

    if (frontSide)
    {
        info.medium_out_id = face.back_medium_id;
    }
    else
    {
        info.medium_out_id = face.front_medium_id;
    }

    info.entered_from_front_side = frontSide;
    info.valid = info.dual_side_semantic_complete && info.medium_in_id >= 0 && info.medium_out_id >= 0;
    return info;
}

} // namespace rt
