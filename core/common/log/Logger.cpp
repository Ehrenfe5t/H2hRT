// 文件目标：
// - 实现模块1统一日志系统。
//
// 主要功能：
// - 初始化控制台与文件日志输出；
// - 按日志级别过滤消息；
// - 输出统一时间戳/级别/run-id/模块名前缀；
// - 为整个系统提供单一日志路径。

#include "Logger.h"

#include <Windows.h>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace rt {

/// <summary>
/// Construct Logger with safe defaults.
/// </summary>
Logger::Logger()
    : consoleEnabled_(true),
      fileEnabled_(false),
      minLevel_(LogLevel::Info),
      runId_("local-run")
{
}

/// <summary>
/// Destroy Logger and close the file stream when needed.
/// </summary>
Logger::~Logger()
{
    if (fileStream_.is_open())
    {
        fileStream_.flush();
        fileStream_.close();
    }
}

/// <summary>
/// Initialize Logger from runtime config.
/// </summary>
/// <param name="runtimeConfig">Runtime config block.</param>
/// <returns>true on full success; false when file logging fails.</returns>
bool Logger::Initialize(const AppRuntimeConfig& runtimeConfig)
{
    consoleEnabled_ = runtimeConfig.enable_console_logging;
    fileEnabled_ = runtimeConfig.enable_file_logging;
    minLevel_ = ParseLevel(runtimeConfig.log_level);
    runId_ = runtimeConfig.run_id;

    if (fileEnabled_)
    {
        EnsureDirectoryForFile(runtimeConfig.log_file_path);
        fileStream_.open(runtimeConfig.log_file_path.c_str(), std::ios::out | std::ios::app);
        if (!fileStream_.is_open())
        {
            fileEnabled_ = false;
            LogError("日志", RtError::Create(
                ErrorCode::LoggerInitializationFailed,
                "日志",
                "Failed to open log file. Logging will continue on console only.",
                runtimeConfig.log_file_path,
                "Check output directory permissions.",
                false));
            return false;
        }
        if (fileStream_.tellp() == std::streampos(0))
        {
            fileStream_ << "\xEF\xBB\xBF";
        }
    }

    Log(LogLevel::Info, "运行时", "日志系统初始化完成。");
    return true;
}

/// <summary>
/// Log a normal message.
/// </summary>
/// <param name="level">Log level.</param>
/// <param name="moduleName">Module name.</param>
/// <param name="message">Message text.</param>
/// <returns>No return value.</returns>
void Logger::Log(LogLevel level, const std::string& moduleName, const std::string& message)
{
    if (!ShouldLog(level))
    {
        return;
    }

    std::ostringstream stream;
    stream << "[" << CurrentTimestamp() << "]"
           << "[" << ToLevelString(level) << "]"
           << "[run=" << runId_ << "]"
           << "[" << moduleName << "] "
           << message;
    WriteLine(stream.str());
}

/// <summary>
/// Log a unified error object.
/// </summary>
/// <param name="moduleName">Module name.</param>
/// <param name="error">Unified error object.</param>
/// <returns>No return value.</returns>
void Logger::LogError(const std::string& moduleName, const RtError& error)
{
    Log(error.fatal ? LogLevel::Fatal : LogLevel::Error, moduleName, error.ToString());
}

/// <summary>
/// Decide whether a message passes the current level filter.
/// </summary>
/// <param name="level">Message level.</param>
/// <returns>true if the message should be emitted.</returns>
bool Logger::ShouldLog(LogLevel level) const
{
    return static_cast<int>(level) >= static_cast<int>(minLevel_);
}

/// <summary>
/// Write one formatted line to all enabled outputs.
/// </summary>
/// <param name="line">Formatted log line.</param>
/// <returns>No return value.</returns>
void Logger::WriteLine(const std::string& line)
{
    if (consoleEnabled_)
    {
        std::cout << line << std::endl;
    }

    if (fileEnabled_ && fileStream_.is_open())
    {
        fileStream_ << line << std::endl;
    }
}

/// <summary>
/// Parse a log level string.
/// </summary>
/// <param name="levelText">Log level text.</param>
/// <returns>Matching log level; defaults to Info when unknown.</returns>
LogLevel Logger::ParseLevel(const std::string& levelText)
{
    if (levelText == "TRACE") return LogLevel::Trace;
    if (levelText == "DEBUG") return LogLevel::Debug;
    if (levelText == "WARN") return LogLevel::Warn;
    if (levelText == "ERROR") return LogLevel::Error;
    if (levelText == "FATAL") return LogLevel::Fatal;
    return LogLevel::Info;
}

/// <summary>
/// Convert a log level enum to text.
/// </summary>
/// <param name="level">Log level enum.</param>
/// <returns>Stable log level text.</returns>
std::string Logger::ToLevelString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Fatal: return "FATAL";
    default: return "INFO";
    }
}

/// <summary>
/// Get the current timestamp string.
/// </summary>
/// <returns>Timestamp in local time.</returns>
std::string Logger::CurrentTimestamp()
{
    const std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

/// <summary>
/// Extract the directory part from a file path.
/// </summary>
/// <param name="filePath">Full file path.</param>
/// <returns>Directory path or an empty string.</returns>
std::string Logger::BuildDirectoryFromFilePath(const std::string& filePath)
{
    const std::size_t pos = filePath.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        return std::string();
    }

    return filePath.substr(0, pos);
}

/// <summary>
/// Create the parent directories for the target log file if needed.
/// </summary>
/// <param name="filePath">Target log file path.</param>
/// <returns>No return value.</returns>
void Logger::EnsureDirectoryForFile(const std::string& filePath)
{
    const std::string directoryPath = BuildDirectoryFromFilePath(filePath);
    if (directoryPath.empty())
    {
        return;
    }

    std::string current;
    for (std::size_t i = 0; i < directoryPath.size(); ++i)
    {
        const char ch = directoryPath[i];
        current.push_back(ch);

        if (ch == '/' || ch == '\\')
        {
            if (current.size() > 1U)
            {
                CreateDirectoryA(current.c_str(), nullptr);
            }
        }
    }

    CreateDirectoryA(directoryPath.c_str(), nullptr);
}

} // namespace rt
