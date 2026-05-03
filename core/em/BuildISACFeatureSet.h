// 文件目标：
// - 声明模块5批次8的通感基础特征构建接口。
//
// 主要功能：
// - 根据路径级 EMPathResultSet 生成第一版 ISACFeatureSet；
// - 输出最早时延、最强路径等基础特征；
// - 为后续更丰富通感分析保留统一入口。

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// 构建通感基础特征结果。
/// </summary>
/// <param name="pathResults">路径级电磁结果集合。</param>
/// <returns>结构化通感基础特征结果。</returns>
ISACFeatureSet BuildISACFeatureSet(const EMPathResultSet& pathResults);

} // namespace rt
