// 文件目标：
// - 声明模块5批次7的发射场初始化接口。
//
// 主要功能：
// - 根据频率与初始路径状态建立 FieldAccumulator；
// - 设置波长、初始复振幅与极化占位信息；
// - 为后续段传播与交互更新提供统一起点。

#pragma once

#include "EMSolverInput.h"
#include "FieldAccumulator.h"

namespace rt {

/// <summary>
/// 初始化路径级发射场状态。
/// </summary>
/// <param name="input">电磁求解输入。</param>
/// <param name="field">待写入的场状态累积器。</param>
/// <returns>true 表示初始化成功；false 表示失败。</returns>
bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field);

} // namespace rt
