// 文件目标：
// - 声明模块5批次7的反射交互更新接口。
//
// 主要功能：
// - 根据反射节点对 FieldAccumulator 做第一版复场更新；
// - 保留后续 Fresnel 精化所需的接口位置；
// - 避免模块5回头重新计算几何成立性。

#pragma once

#include "FieldAccumulator.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// 对反射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前反射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node);

} // namespace rt
