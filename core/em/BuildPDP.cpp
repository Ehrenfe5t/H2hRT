// 文件目标：
// - 实现模块5批次8的 PDP 构建逻辑。
//
// 主要功能：
// - 将路径级时延与功率映射为 PDP 抽头；
// - 保持结构简单明确，便于后续增强；
// - 为批次8的 PDP 闭环提供基础输出。

#include "BuildPDP.h"

namespace rt {

/// <summary>
/// 构建 PDP 结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化 PDP 结果。</returns>
PDPResult BuildPDP(const EMPathResultSet& pathResults)
{
    PDPResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid)
        {
            continue;
        }
        PDPTap tap;
        tap.delay_s = item.delay_s;
        tap.power_linear = item.power_linear;
        result.taps.push_back(tap);
    }
    return result;
}

} // namespace rt
