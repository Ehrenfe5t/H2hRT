// 文件目标：
// - 定义系统版本信息对象。
//
// 主要功能：
// - 统一管理程序版本、配置结构版本、预处理版本、缓存格式版本和验证规则版本；
// - 为后续 cache 兼容性判断、结果追踪和回归复现提供稳定版本入口。

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 系统版本信息集合。
/// </summary>
struct VersionInfo {
    std::string program_version;
    std::string config_schema_version;
    std::string preprocess_algorithm_version;
    std::string scene_cache_format_version;
    std::string validation_rule_version;

    /// <summary>
    /// 获取当前程序编译期约定的版本信息。
    /// </summary>
    /// <returns>当前版本信息对象。</returns>
    static VersionInfo Current();

    /// <summary>
    /// 将版本信息格式化为可读字符串。
    /// </summary>
    /// <returns>便于日志输出和调试查看的版本摘要字符串。</returns>
    std::string ToString() const;
};

} // namespace rt
