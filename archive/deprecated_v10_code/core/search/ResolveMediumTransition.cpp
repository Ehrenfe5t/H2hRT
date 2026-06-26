// 文件目标：
// - 实现模块4批次6的透射介质切换解析逻辑。
//
// 主要功能：
// - 根据面法向与当前介质状态确定 medium in/out；
// - 在批次6中优先保证显式结果与失败可诊断；
// - 为后续模块5 transmission 电磁求解提供稳定语义基础。

#include "ResolveMediumTransition.h"
#include "../common/math/Vec3.h"

namespace rt {

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
    info.entered_from_front_side = frontSide;

    // v9 step27: 介质侧匹配校验 — 当前介质必须等于face的入射侧介质
    int expected_in = frontSide ? face.front_medium_id : face.back_medium_id;
    int expected_out = frontSide ? face.back_medium_id : face.front_medium_id;

    if (state.current_medium_id != expected_in) {
        // 介质不匹配: 拒绝该候选 (strict模式)
        info.medium_in_id = state.current_medium_id;
        info.medium_out_id = expected_out;
        info.valid = false; // medium mismatch → invalid
        return info;
    }

    info.medium_in_id = expected_in;
    info.medium_out_id = expected_out;
    info.valid = info.dual_side_semantic_complete && info.medium_in_id >= 0 && info.medium_out_id >= 0;
    return info;
}

} // namespace rt
