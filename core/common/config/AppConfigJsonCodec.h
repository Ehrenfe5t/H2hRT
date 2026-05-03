// 文件目标：
// - 声明模块1正式使用的 JSON 编解码接口。
//
// 主要功能：
// - 将 JSON 文件解析为 AppConfig；
// - 将 AppConfig 序列化为配置快照 JSON；
// - 集中维护配置字段映射，保持加载器与快照写出逻辑一致。

#pragma once

#include "AppConfig.h"

#include <string>

namespace rt {

/// <summary>
/// Structured JSON decode result for AppConfig.
/// </summary>
struct AppConfigJsonDecodeResult {
    bool succeeded = false;
    AppConfig config;
    std::string error_message;
};

/// <summary>
/// Decode AppConfig from a JSON file using the formal module-1 JSON path.
/// </summary>
/// <param name="filePath">Input JSON config file path.</param>
/// <returns>Decode result containing success state, config object and error message.</returns>
AppConfigJsonDecodeResult DecodeAppConfigFromJsonFile(const std::string& filePath);

/// <summary>
/// Encode AppConfig into JSON text.
/// </summary>
/// <param name="config">Configuration object to serialize.</param>
/// <returns>Formatted JSON string.</returns>
std::string EncodeAppConfigToJsonString(const AppConfig& config);

} // namespace rt
