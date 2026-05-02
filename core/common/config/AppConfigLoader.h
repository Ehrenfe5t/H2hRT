// 文件目标：
// - 声明模块1配置加载器接口。
//
// 主要功能：
// - 从 JSON 配置文件构建 AppConfig；
// - 在加载阶段显式返回统一错误对象；
// - 为 app 层提供“加载成功/失败 + 默认配置回落对象 + 错误列表”的结构化结果。

#pragma once

#include "AppConfig.h"
#include "../error/RtError.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// AppConfig 文件加载结果。
/// </summary>
struct AppConfigLoadResult {
    bool load_succeeded = false;
    AppConfig config;
    std::string source_file_path;
    std::vector<RtError> errors;
};

/// <summary>
/// 从 JSON 文件加载应用配置。
/// </summary>
/// <param name="filePath">JSON 配置文件路径。</param>
/// <returns>
/// 结构化加载结果：
/// - load_succeeded=true 表示读取与基础解析完成；
/// - config 为解析后的配置对象，若字段缺失则保留默认值；
/// - errors 为加载阶段显式产生的错误集合。
/// </returns>
AppConfigLoadResult LoadAppConfigFromJsonFile(const std::string& filePath);

} // namespace rt
