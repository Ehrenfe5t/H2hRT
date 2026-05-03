// 文件目标：
// - 声明模块5批次7的绕射交互更新接口。
//
// 主要功能：
// - 根据绕射节点对 FieldAccumulator 做第一版复场更新；
// - 为后续 UTD 绕射系数精化预留接口位置；
// - 保持模块5主链结构完整。

#pragma once

#include "FieldAccumulator.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// 对绕射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前绕射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node);

} // namespace rt
