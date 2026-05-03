// 文件目标：
// - 实现模块5批次8的 PreciseEM profile 构建逻辑。
//
// 主要功能：
// - 指定完整复场、相位与极化保留策略；
// - 保证批次8可切换到 PreciseEM 模式；
// - 为后续更精细 profile 参数扩展预留入口。

#include "PreciseEMProfile.h"

namespace rt {

/// <summary>
/// 构建 PreciseEM profile。
/// </summary>
/// <returns>结构化 PreciseEM profile。</returns>
EMSolveProfile BuildPreciseEMProfile()
{
    EMSolveProfile profile;
    profile.mode = EMSolveMode::PreciseEM;
    profile.keep_full_complex_field = true;
    profile.keep_full_polarization = true;
    profile.keep_phase_output = true;
    profile.enable_receiver_polarization_projection = true;
    profile.enable_coherent_sum = true;
    profile.enable_noncoherent_power_sum = false;
    profile.max_paths_per_receiver = 64;
    profile.min_power_threshold_linear = 0.0;
    return profile;
}

} // namespace rt
