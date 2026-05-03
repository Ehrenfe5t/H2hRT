// 文件目标：
// - 实现模块5批次8的 CIR 构建逻辑。
//
// 主要功能：
// - 将路径级时延与复振幅映射为 CIR 抽头；
// - 根据 profile 选择相干或非相干表达；
// - 为批次8的时延域结果闭环提供基础输出。

#include "BuildCIR.h"

namespace rt {

/// <summary>
/// 构建 CIR 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <param name="profile">当前求值 profile。</param>
/// <returns>结构化 CIR 结果。</returns>
CIRResult BuildCIR(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    CIRResult result;
    result.coherent = profile.enable_coherent_sum;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid || item.power_linear < profile.min_power_threshold_linear)
        {
            continue;
        }
        CIRTap tap;
        tap.delay_s = item.delay_s;
        tap.amplitude_real = item.amplitude_real;
        tap.amplitude_imag = item.amplitude_imag;
        tap.power_linear = item.power_linear;
        tap.contributing_path_ids.push_back(item.path_id);
        result.taps.push_back(tap);
    }
    return result;
}

} // namespace rt
