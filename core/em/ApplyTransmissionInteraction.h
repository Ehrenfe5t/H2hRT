// 文件目标：
// - 声明模块5批次7的透射交互更新接口。
//
// 主要功能：
// - 根据透射节点对 FieldAccumulator 做第一版复场更新；
// - 保留后续 Fresnel 透射系数精化接口；
// - 显式更新当前介质状态。

#pragma once

#include "FieldAccumulator.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// 对透射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前透射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node);

} // namespace rt
