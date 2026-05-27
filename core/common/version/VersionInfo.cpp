// 文件目标：
// - 实现系统版本信息的默认生成与字符串输出。
//
// 主要功能：
// - 提供当前批次版本快照；
// - 统一输出版本摘要，供日志与后续结果追踪使用。

#include "VersionInfo.h"

#include <sstream>

namespace rt {

/// <summary>
/// 返回当前代码版本快照。
/// </summary>
/// <returns>当前批次使用的 VersionInfo 对象。</returns>
VersionInfo VersionInfo::Current()
{
    VersionInfo version;
    version.program_version = "0.1.0-batch01";
    version.config_schema_version = "1.0.0";
    version.preprocess_algorithm_version = "1.0.0-placeholder";
    version.scene_cache_format_version = "1.0.0-placeholder";
    version.validation_rule_version = "1.0.0-placeholder";
    return version;
}

/// <summary>
/// 将版本信息对象转成简洁摘要字符串。
/// </summary>
/// <returns>版本摘要字符串。</returns>
std::string VersionInfo::ToString() const
{
    std::ostringstream stream;
    stream << "program=" << program_version
           << ", config_schema=" << config_schema_version
           << ", preprocess=" << preprocess_algorithm_version
           << ", scene_cache=" << scene_cache_format_version
           << ", validation=" << validation_rule_version;
    return stream.str();
}

} // namespace rt
