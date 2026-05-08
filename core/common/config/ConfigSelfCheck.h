// 文件目标：
// - 声明配置自检入口，用于完成批次0/1的显式验证闭环。
//
// 主要功能：
// - 复跑已知负样例配置；
// - 确保关键校验规则持续阻断非法启动状态；
// - 为流水线与日志提供结构化自检结果。

#pragma once

#include "AppConfig.h"
#include "../error/RtError.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// Result of module-1 self-check execution.
/// </summary>
struct ConfigSelfCheckResult {
    bool succeeded = false;
    std::vector<std::string> details;
    RtError error;
};

/// <summary>
/// Execute module-1 self-check cases.
/// </summary>
/// <param name="config">Current validated application config.</param>
/// <returns>Structured self-check result.</returns>
ConfigSelfCheckResult RunModule1SelfCheck(const AppConfig& config);

} // namespace rt
