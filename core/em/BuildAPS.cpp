// 文件目标：
// - 实现模块5批次8的 APS 构建逻辑。
//
// 主要功能：
// - 使用当前批次可得的基础极化/方向占位构建角域摘要；
// - 保证批次8可生成 APSResult；
// - 为后续更精细角度建模保留可替换位置。

#include "BuildAPS.h"

#include <cmath>

namespace rt {

/// <summary>
/// 构建 APS 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化 APS 结果。</returns>
APSResult BuildAPS(const EMPathResultSet& pathResults)
{
    APSResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue;
        }
        APSEntry entry;
        entry.angle_metric = item.polarization_vector.x;
        entry.power_linear = item.power_linear;
        result.entries.push_back(entry);
    }
    return result;
}

} // namespace rt
