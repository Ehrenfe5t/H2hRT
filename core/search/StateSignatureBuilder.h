// 文件目标：
// - 声明模块4状态级去重使用的签名构建接口。
//
// 主要功能：
// - 为 PathState 生成稳定字符串签名；
// - 支持批次5骨架阶段的状态级去重；
// - 为后续批次增强量化策略与容差去重保留统一入口。

#pragma once

#include "../common/config/AppConfig.h"
#include "../path/PathState.h"

#include <string>

namespace rt {

/// <summary>
/// 根据 PathState 构建状态签名。
/// </summary>
/// <param name="state">待构建签名的路径状态。</param>
/// <param name="config">统一应用配置对象。</param>
/// <returns>稳定状态签名字符串。</returns>
std::string BuildStateSignature(const PathState& state, const AppConfig& config);

} // namespace rt
