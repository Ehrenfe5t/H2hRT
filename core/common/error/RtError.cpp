// 文件目标：
// - 实现模块1统一错误对象的编码转换与字符串格式化逻辑。
//
// 主要功能：
// - 将 ErrorCode 映射到稳定错误码字符串；
// - 构造 RtError；
// - 输出可直接写入日志和报告的格式化字符串。

#include "RtError.h"

#include <sstream>

namespace rt {

/// <summary>
/// 将错误码枚举映射为稳定错误码字符串。
/// </summary>
/// <param name="code">错误码枚举值。</param>
/// <returns>统一错误码字符串。</returns>
std::string ToErrorCodeString(ErrorCode code)
{
    switch (code)
    {
    case ErrorCode::None:
        return "NONE-000";
    case ErrorCode::ConfigMissing:
        return "CFG-001";
    case ErrorCode::ConfigInvalidRange:
        return "CFG-002";
    case ErrorCode::FileNotFound:
        return "SCN-001";
    case ErrorCode::JsonParseError:
        return "CFG-003";
    case ErrorCode::ValidationFailed:
        return "VAL-001";
    case ErrorCode::LoggerInitializationFailed:
        return "LOG-001";
    case ErrorCode::SelfCheckFailed:
        return "CHK-001";
    case ErrorCode::InternalError:
        return "SYS-001";
    default:
        return "UNK-999";
    }
}

/// <summary>
/// 创建统一错误对象。
/// </summary>
/// <param name="code">错误码枚举值。</param>
/// <param name="moduleName">错误所属模块名。</param>
/// <param name="message">错误描述。</param>
/// <param name="context">详细上下文。</param>
/// <param name="suggestion">建议排查方向。</param>
/// <param name="fatal">是否致命。</param>
/// <returns>构造完成的 RtError 对象。</returns>
RtError RtError::Create(
    ErrorCode code,
    const std::string& moduleName,
    const std::string& message,
    const std::string& context,
    const std::string& suggestion,
    bool fatal)
{
    RtError error;
    error.code = code;
    error.code_string = ToErrorCodeString(code);
    error.module_name = moduleName;
    error.message = message;
    error.context = context;
    error.suggestion = suggestion;
    error.fatal = fatal;
    return error;
}

/// <summary>
/// 将统一错误对象格式化为日志友好字符串。
/// </summary>
/// <returns>可直接写入日志、终端或调试输出的错误文本。</returns>
std::string RtError::ToString() const
{
    std::ostringstream stream;
    stream << "[" << code_string << "] "
           << "module=" << module_name
           << ", fatal=" << (fatal ? "true" : "false")
           << ", message=" << message;

    if (!context.empty())
    {
        stream << ", context=" << context;
    }

    if (!suggestion.empty())
    {
        stream << ", suggestion=" << suggestion;
    }

    return stream.str();
}

} // namespace rt
