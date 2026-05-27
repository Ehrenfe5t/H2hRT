// 文件目标：
// - 声明模块1统一配置校验入口。
//
// 主要功能：
// - 对 AppConfig 执行字段级、块内和跨块一致性检查；
// - 统一返回 ConfigValidationResult，避免后续模块零散判空和重复校验。

#pragma once

#include "AppConfig.h"

namespace rt {

/// <summary>
/// 对统一应用配置执行模块1级别的正式校验。
/// </summary>
/// <param name="config">待校验的统一应用配置对象。</param>
/// <returns>配置校验结果，包含 passed、warnings 和 errors。</returns>
ConfigValidationResult ValidateAppConfig(const AppConfig& config);

} // namespace rt
