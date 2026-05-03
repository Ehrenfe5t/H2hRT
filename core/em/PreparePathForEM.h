// 文件目标：
// - 声明模块5批次7的路径预整理入口。
//
// 主要功能：
// - 对 GeometricPath 做最小合法性复核；
// - 统一为后续求值子过程提供已校验路径对象；
// - 阻断关键空路径或无效节点进入物理主链。

#pragma once

#include "EMSolverInput.h"

namespace rt {

/// <summary>
/// 对路径做进入模块5前的最小预检查。
/// </summary>
/// <param name="input">电磁求解输入。</param>
/// <returns>true 表示路径可进入模块5主链；false 表示应阻断。</returns>
bool PreparePathForEM(const EMSolverInput& input);

} // namespace rt
