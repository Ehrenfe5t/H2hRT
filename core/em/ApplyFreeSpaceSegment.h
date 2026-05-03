// 文件目标：
// - 声明模块5批次7的自由空间段传播接口。
//
// 主要功能：
// - 按段长度累积时延、相位和路径损耗；
// - 更新 FieldAccumulator 中的复振幅；
// - 为交互前后的段传播建立统一主干。

#pragma once

#include "FieldAccumulator.h"

namespace rt {

/// <summary>
/// 对单段自由空间传播更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="segmentLengthM">当前段长度。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyFreeSpaceSegment(FieldAccumulator& field, double segmentLengthM);

} // namespace rt
