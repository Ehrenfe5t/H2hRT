// 文件目标：
// - 定义模块1及后续模块共用的统一错误码枚举。
//
// 主要功能：
// - 为 RtError 提供稳定的类型化错误码来源；
// - 避免各头文件重复定义错误码；
// - 为后续模块扩展保持集中且兼容的错误码体系。

#pragma once

namespace rt {

/// <summary>
/// Unified system error code enumeration.
/// </summary>
enum class ErrorCode {
    None = 0,
    ConfigMissing,
    ConfigInvalidRange,
    FileNotFound,
    JsonParseError,
    ValidationFailed,
    LoggerInitializationFailed,
    SelfCheckFailed,
    InternalError
};

} // namespace rt
