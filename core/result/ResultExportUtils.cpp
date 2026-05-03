// 文件目标：
// - 实现模块6批次9导出辅助函数。
//
// 主要功能：
// - 创建目录层级；
// - 写出 UTF-8 文本文件；
// - 为 JSON/CSV/报告导出提供统一底层工具。

#include "ResultExportUtils.h"

#include <Windows.h>

#include <fstream>

namespace rt {

/// <summary>
/// 确保指定目录存在。
/// </summary>
/// <param name="directoryPath">目标目录路径。</param>
/// <returns>true 表示目录可用；false 表示失败。</returns>
bool EnsureResultDirectory(const std::string& directoryPath)
{
    if (directoryPath.empty())
    {
        return false;
    }

    std::string current;
    for (std::size_t i = 0; i < directoryPath.size(); ++i)
    {
        current.push_back(directoryPath[i]);
        if (directoryPath[i] == '/' || directoryPath[i] == '\\')
        {
            if (current.size() > 1U)
            {
                CreateDirectoryA(current.c_str(), nullptr);
            }
        }
    }
    return CreateDirectoryA(directoryPath.c_str(), nullptr) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
}

/// <summary>
/// 将文本内容写入指定文件。
/// </summary>
/// <param name="filePath">目标文件路径。</param>
/// <param name="content">待写出的文本内容。</param>
/// <returns>true 表示写出成功；false 表示失败。</returns>
bool WriteTextFile(const std::string& filePath, const std::string& content)
{
    std::ofstream output(filePath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!output.is_open())
    {
        return false;
    }
    output << content;
    output.flush();
    return output.good();
}

} // namespace rt
