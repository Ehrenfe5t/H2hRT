// 文件目标：
// - 实现模块5批次8的 CoverageEM profile 构建逻辑。
//
// 主要功能：
// - 指定覆盖仿真模式下的简化保留策略；
// - 保证批次8可切换到 CoverageEM 模式；
// - 为后续大规模覆盖仿真优化保留扩展入口。

#include "CoverageEMProfile.h"

namespace rt {

/// <summary>
/// 构建 CoverageEM profile。
/// </summary>
/// <returns>结构化 CoverageEM profile。</returns>
EMSolveProfile BuildCoverageEMProfile()
{
    EMSolveProfile profile;
    profile.mode = EMSolveMode::CoverageEM;
    profile.keep_full_complex_field = false;
    profile.keep_full_polarization = false;
    profile.keep_phase_output = false;
    profile.enable_receiver_polarization_projection = false;
    profile.enable_coherent_sum = false;
    profile.enable_noncoherent_power_sum = true;
    profile.max_paths_per_receiver = 16;
    profile.min_power_threshold_linear = 1.0e-6;
    return profile;
}

} // namespace rt
