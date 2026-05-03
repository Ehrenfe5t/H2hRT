// 文件目标：
// - 声明模块5批次7的接收端收敛接口。
//
// 主要功能：
// - 将 FieldAccumulator 收敛为单条 EMPathResult；
// - 输出时延、相位、复振幅和功率；
// - 作为模块5路径级主链的终点。

#pragma once

#include "EMPathResult.h"
#include "FieldAccumulator.h"
#include "../path/GeometricPath.h"

namespace rt {

/// <summary>
/// 将路径级场状态收敛为接收端结果。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="path">当前几何路径。</param>
/// <returns>单条路径级电磁结果。</returns>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path);

} // namespace rt
