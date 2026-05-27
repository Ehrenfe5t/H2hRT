// 文件目标：
// - 定义模块1统一错误码与错误对象。
//
// 主要功能：
// - 为各模块提供统一错误表达；
// - 支持错误码、模块名、上下文、建议排查方向等诊断信息；
// - 供 Logger、启动流程和后续验证报告统一消费。

#pragma once

#include "ErrorCode.h"

#include <string>

namespace rt {

/// <summary>
/// 系统统一错误对象。
/// </summary>
struct RtError {
    ErrorCode code = ErrorCode::None;
    std::string code_string;
    std::string module_name;
    std::string message;
    std::string context;
    std::string suggestion;
    bool fatal = false;

    /// <summary>
    /// 构造统一错误对象。
    /// </summary>
    /// <param name="code">统一错误码枚举值。</param>
    /// <param name="moduleName">错误所属模块名。</param>
    /// <param name="message">人可读错误描述。</param>
    /// <param name="context">定位上下文，如文件路径、对象标识等。</param>
    /// <param name="suggestion">建议排查方向。</param>
    /// <param name="fatal">是否属于致命错误。</param>
    /// <returns>填充好的 RtError 对象。</returns>
    static RtError Create(
        ErrorCode code,
        const std::string& moduleName,
        const std::string& message,
        const std::string& context,
        const std::string& suggestion,
        bool fatal);

    /// <summary>
    /// 将错误对象格式化为可输出字符串。
    /// </summary>
    /// <returns>包含错误码、模块名、严重性、描述与上下文的字符串。</returns>
    std::string ToString() const;
};

/// <summary>
/// 将统一错误码枚举映射为稳定的字符串编码。
/// </summary>
/// <param name="code">错误码枚举值。</param>
/// <returns>如 CFG-001、VAL-001 之类的稳定编码。</returns>
std::string ToErrorCodeString(ErrorCode code);

} // namespace rt
