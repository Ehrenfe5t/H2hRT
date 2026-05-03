// 文件目标：
// - 实现模块5批次8的信道统计构建逻辑。
//
// 主要功能：
// - 汇总有效路径数、总功率、最强路径功率和平均时延；
// - 保证批次8可输出基础统计量；
// - 为后续更丰富统计指标扩展保留空间。

#include "BuildChannelStatistics.h"

namespace rt {

/// <summary>
/// 构建信道统计结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化信道统计结果。</returns>
ChannelStatistics BuildChannelStatistics(const EMPathResultSet& pathResults)
{
    ChannelStatistics result;
    double delaySum = 0.0;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue;
        }
        ++result.valid_path_count;
        result.total_power_linear += item.power_linear;
        if (item.power_linear > result.strongest_path_power_linear)
        {
            result.strongest_path_power_linear = item.power_linear;
        }
        delaySum += item.delay_s;
    }
    if (result.valid_path_count > 0)
    {
        result.mean_delay_s = delaySum / static_cast<double>(result.valid_path_count);
    }
    return result;
}

} // namespace rt
