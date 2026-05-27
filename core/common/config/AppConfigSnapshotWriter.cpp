// 文件目标：
// - 实现 AppConfig 配置快照导出功能。
//
// 主要功能：
// - 把当前运行时配置转换为可审查 JSON 文本；
// - 在输出目录中按 run_id 生成稳定文件名；
// - 显式返回写出失败原因，而不是静默忽略配置快照导出失败。

#include "AppConfigSnapshotWriter.h"

#include "AppConfigJsonCodec.h"

#include <Windows.h>

#include <fstream>

namespace rt {

namespace {

/// <summary>
/// 从完整文件路径中提取目录部分。
/// </summary>
/// <param name="filePath">完整文件路径。</param>
/// <returns>目录路径；若不存在则返回空字符串。</returns>
std::string BuildDirectoryFromFilePath(const std::string& filePath)
{
    const std::size_t pos = filePath.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        return std::string();
    }

    return filePath.substr(0, pos);
}

/// <summary>
/// 为目标文件创建必要目录。
/// </summary>
/// <param name="filePath">目标文件路径。</param>
/// <returns>无返回值。</returns>
void EnsureDirectoryForFile(const std::string& filePath)
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


} // namespace

/// <summary>
/// 将 AppConfig 导出为配置快照文件。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <returns>快照写出结果。</returns>
AppConfigSnapshotWriteResult WriteAppConfigSnapshot(const AppConfig& config)
{
    AppConfigSnapshotWriteResult result;
    result.output_file_path = config.app_runtime.config_snapshot_directory + "/" + config.app_runtime.run_id + "_app_config_snapshot.json";

    EnsureDirectoryForFile(result.output_file_path);

    std::ofstream output(result.output_file_path.c_str(), std::ios::out | std::ios::trunc);
    if (!output.is_open())
    {
        result.error = RtError::Create(
            ErrorCode::InternalError,
            "Module1",
            "Failed to open config snapshot output file.",
            result.output_file_path,
            "Check snapshot directory existence and write permissions.",
            true);
        return result;
    }

    output << EncodeAppConfigToJsonString(config);
    output.flush();

    if (!output.good())
    {
        result.error = RtError::Create(
            ErrorCode::InternalError,
            "Module1",
            "Failed while writing config snapshot output file.",
            result.output_file_path,
            "Check disk availability and output path permissions.",
            true);
        return result;
    }

    result.succeeded = true;
    return result;
}

} // namespace rt
