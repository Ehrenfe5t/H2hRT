// 文件目标：
// - 声明模块6批次9导出辅助函数。
//
// 主要功能：
// - 创建导出目录；
// - 写出文本文件；
// - 提供 JSON/CSV 导出阶段复用的基础工具。

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 确保指定目录存在。
/// </summary>
/// <param name="directoryPath">目标目录路径。</param>
/// <returns>true 表示目录可用；false 表示失败。</returns>
bool EnsureResultDirectory(const std::string& directoryPath);

/// <summary>
/// 将文本内容写入指定文件。
/// </summary>
/// <param name="filePath">目标文件路径。</param>
/// <param name="content">待写出的文本内容。</param>
/// <returns>true 表示写出成功；false 表示失败。</returns>
bool WriteTextFile(const std::string& filePath, const std::string& content);

} // namespace rt
