// 文件目标：
// - 实现模块4状态级签名构建逻辑。
//
// 主要功能：
// - 将 PathState 的关键几何和预算字段量化为稳定签名；
// - 为批次5骨架阶段提供最小状态去重能力；
// - 避免后续模块4实现阶段重复散落签名拼接逻辑。

#include "StateSignatureBuilder.h"

#include <cmath>
#include <sstream>

namespace rt {

namespace {

long long Quantize(double value, double eps)
{
    const double safeEps = (eps > 0.0) ? eps : 1.0e-6;
    return static_cast<long long>(std::llround(value / safeEps));
}

} // namespace

/// <summary>
/// 根据 PathState 构建状态签名。
/// </summary>
/// <param name="state">待构建签名的路径状态。</param>
/// <param name="config">统一应用配置对象。</param>
/// <returns>稳定状态签名字符串。</returns>
std::string BuildStateSignature(const PathState& state, const AppConfig& config)
{
    std::ostringstream stream;
    stream << static_cast<int>(state.last_interaction_type) << "|"
           << state.path_depth << "|"
           << state.last_hit_face_id << "|"
           << state.last_hit_wedge_id << "|"
           << Quantize(state.current_point.x, config.numeric_tolerance.eps_deduplicate) << ","
           << Quantize(state.current_point.y, config.numeric_tolerance.eps_deduplicate) << ","
           << Quantize(state.current_point.z, config.numeric_tolerance.eps_deduplicate) << "|"
           << Quantize(state.current_direction.x, config.numeric_tolerance.eps_angle) << ","
           << Quantize(state.current_direction.y, config.numeric_tolerance.eps_angle) << ","
           << Quantize(state.current_direction.z, config.numeric_tolerance.eps_angle) << "|"
           << state.remaining_total_expansions << "|"
           << state.remaining_reflections << "|"
           << state.remaining_transmissions << "|"
           << state.remaining_diffractions;
    return stream.str();
}

} // namespace rt
