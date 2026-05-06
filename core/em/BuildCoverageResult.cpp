// 文件目标：
// - 实现模块5批次8的覆盖结果构建逻辑。
//
// 主要功能：
// - 汇总 CoverageEM 关注的总接收功率与贡献路径数；
// - 保证批次8可输出第一版 CoverageResult；
// - 为后续网格化 coverage 扩展预留接口。

#include "BuildCoverageResult.h"

namespace rt {

/// <summary>
/// 构建覆盖结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <param name="profile">当前求值 profile。</param>
/// <returns>结构化覆盖结果。</returns>
CoverageResult BuildCoverageResult(const EMPathResultSet& pathResults, const EMSolveProfile& profile)
{
    CoverageResult result;
    double lossSum = 0.0;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid || item.power_linear < profile.min_power_threshold_linear)
        {
            continue;
        }
        result.total_received_power_linear += item.power_linear;
        ++result.contributing_path_count;
        lossSum += item.free_space_loss_db;
    }
    if (result.contributing_path_count > 0)
    {
        result.average_free_space_loss_db = lossSum / static_cast<double>(result.contributing_path_count);
    }
    return result;
}

} // namespace rt
