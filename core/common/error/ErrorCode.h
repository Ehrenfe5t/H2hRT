// File purpose:
// - Define the unified error code enumeration shared by module 1 and later modules.
//
// Main responsibilities:
// - Provide a stable typed error-code source for RtError.
// - Avoid duplicating error code definitions across headers.
// - Keep later module expansion compatible with one centralized enumeration.

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
