// 文件目标：
// - 定义模块1统一日志系统接口。
//
// 主要功能：
// - 提供控制台与文件双通道日志输出；
// - 提供统一日志级别与统一格式；
// - 为整个系统禁止随意 printf/std::cout 提供公共替代入口。

#pragma once

#include "../config/AppConfig.h"
#include "../error/RtError.h"

#include <fstream>
#include <string>

namespace rt {

/// <summary>
/// 统一日志级别枚举。
/// </summary>
enum class LogLevel {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

/// <summary>
/// 统一日志输出器。
/// </summary>
class Logger {
public:
    /// <summary>构造日志对象并设置安全默认值。</summary>
    Logger();

    /// <summary>析构日志对象并在需要时关闭文件句柄。</summary>
    ~Logger();

    /// <summary>
    /// 根据运行时配置初始化日志系统。
    /// </summary>
    /// <param name="runtimeConfig">来自 AppConfig 的运行时配置块。</param>
    /// <returns>true 表示初始化完全成功；false 表示文件日志初始化失败但仍可能保留控制台日志。</returns>
    bool Initialize(const AppRuntimeConfig& runtimeConfig);

    /// <summary>
    /// 输出一条普通日志消息。
    /// </summary>
    /// <param name="level">日志级别。</param>
    /// <param name="moduleName">日志所属模块名。</param>
    /// <param name="message">日志正文。</param>
    /// <returns>无返回值。</returns>
    void Log(LogLevel level, const std::string& moduleName, const std::string& message);

    /// <summary>
    /// 输出一条统一错误对象日志。
    /// </summary>
    /// <param name="moduleName">记录日志时使用的模块名。</param>
    /// <param name="error">统一错误对象。</param>
    /// <returns>无返回值。</returns>
    void LogError(const std::string& moduleName, const RtError& error);

private:
    /// <summary>判断指定级别是否应当被当前日志配置输出。</summary>
    /// <param name="level">待判断日志级别。</param>
    /// <returns>true 表示应输出；false 表示应过滤。</returns>
    bool ShouldLog(LogLevel level) const;

    /// <summary>将完整格式化日志行写入控制台和/或文件。</summary>
    /// <param name="line">已格式化完成的日志行。</param>
    /// <returns>无返回值。</returns>
    void WriteLine(const std::string& line);

    /// <summary>把日志级别字符串解析为枚举值。</summary>
    /// <param name="levelText">日志级别文本。</param>
    /// <returns>对应的日志级别枚举；无法识别时默认返回 Info。</returns>
    static LogLevel ParseLevel(const std::string& levelText);

    /// <summary>把日志级别枚举转成稳定字符串。</summary>
    /// <param name="level">日志级别枚举值。</param>
    /// <returns>日志级别字符串。</returns>
    static std::string ToLevelString(LogLevel level);

    /// <summary>获取当前本地时间戳字符串。</summary>
    /// <returns>形如 YYYY-MM-DD HH:MM:SS 的时间字符串。</returns>
    static std::string CurrentTimestamp();

    /// <summary>从日志文件路径中提取目录部分。</summary>
    /// <param name="filePath">完整文件路径。</param>
    /// <returns>目录路径；若无目录部分则返回空字符串。</returns>
    static std::string BuildDirectoryFromFilePath(const std::string& filePath);

    /// <summary>确保日志文件所在目录已存在。</summary>
    /// <param name="filePath">目标日志文件路径。</param>
    /// <returns>无返回值。</returns>
    static void EnsureDirectoryForFile(const std::string& filePath);

private:
    bool consoleEnabled_;
    bool fileEnabled_;
    LogLevel minLevel_;
    std::string runId_;
    std::ofstream fileStream_;
};

} // namespace rt
