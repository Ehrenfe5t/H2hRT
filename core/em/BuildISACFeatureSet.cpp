// 文件目标：
// - 实现模块5批次8的通感基础特征构建逻辑。
//
// 主要功能：
// - 汇总路径数、最早时延与最强路径功率；
// - 保证批次8可输出第一版 ISACFeatureSet；
// - 为后续更复杂通感特征扩展留出空间。

#include "BuildISACFeatureSet.h"

namespace rt {

/// <summary>
/// 构建通感基础特征结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化通感基础特征结果。</returns>
ISACFeatureSet BuildISACFeatureSet(const EMPathResultSet& pathResults)
{
    ISACFeatureSet result;
    bool first = true;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue;
        }
        ++result.path_count;
        if (first || item.delay_s < result.earliest_delay_s)
        {
            result.earliest_delay_s = item.delay_s;
        }
        if (item.power_linear > result.strongest_path_power_linear)
        {
            result.strongest_path_power_linear = item.power_linear;
        }
        first = false;
    }
    return result;
}

} // namespace rt
