// 文件目标：
// - 声明模块4路径级去重使用的签名构建接口。
//
// 主要功能：
// - 为 GeometricPath 生成稳定字符串签名；
// - 支持批次5中 LOS 路径最小闭环的路径级去重；
// - 为后续混合路径扩展保留统一签名入口。

#pragma once

#include "../common/config/AppConfig.h"
#include "../path/GeometricPath.h"

#include <string>

namespace rt {

/// <summary>
/// 根据 GeometricPath 构建路径签名。
/// </summary>
/// <param name="path">待构建签名的几何路径。</param>
/// <param name="config">统一应用配置对象。</param>
/// <returns>稳定路径签名字符串。</returns>
std::string BuildPathSignature(const GeometricPath& path, const AppConfig& config);

} // namespace rt
