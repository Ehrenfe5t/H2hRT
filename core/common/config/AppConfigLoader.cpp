// 文件目标：
// - 基于统一 JSON codec 实现模块1配置加载器。
//
// 主要功能：
// - 调用集中式 JSON 解码层读取配置文件；
// - 在加载失败时返回默认配置对象与显式 RtError；
// - 为 app 层启动流程提供稳定的配置装载边界。

#include "AppConfigLoader.h"

#include "AppConfigJsonCodec.h"
#include "V11UserConfigAdapter.h"

#include <fstream>

namespace rt {

namespace {

/// <summary>
/// 判断指定文件是否存在且可打开。
/// </summary>
/// <param name="filePath">待检查路径。</param>
/// <returns>若文件可访问则返回 true，否则返回 false。</returns>
bool FileExists(const std::string& filePath)
{
    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    return input.good();
}

} // namespace

/// <summary>
/// 从 JSON 文件加载 AppConfig。
/// </summary>
/// <param name="filePath">JSON 配置文件路径。</param>
/// <returns>结构化配置加载结果。</returns>
AppConfigLoadResult LoadAppConfigFromJsonFile(const std::string& filePath)
{
    AppConfigLoadResult result;
    result.source_file_path = filePath;

    if (!FileExists(filePath))
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::FileNotFound,
            "Module1",
            "Unable to open config file.",
            filePath,
            "Check whether the JSON config path exists.",
            true));
        return result;
    }

    const V11UserConfigDecodeResult v11Result = DecodeV11UserConfigFromJsonFile(filePath);
    if (v11Result.recognized)
    {
        result.config = v11Result.config;
        if (!v11Result.succeeded)
        {
            result.errors.push_back(RtError::Create(
                ErrorCode::JsonParseError,
                "Module1",
                "Failed to parse v11 user config file.",
                filePath,
                v11Result.error_message,
                true));
            return result;
        }
        result.load_succeeded = true;
        return result;
    }

    const AppConfigJsonDecodeResult decodeResult = DecodeAppConfigFromJsonFile(filePath);
    result.config = decodeResult.config;

    if (!decodeResult.succeeded)
    {
        result.errors.push_back(RtError::Create(
            ErrorCode::JsonParseError,
            "Module1",
            "Failed to parse JSON config file.",
            filePath,
            decodeResult.error_message,
            true));
        return result;
    }

    result.load_succeeded = true;
    return result;
}

} // namespace rt
