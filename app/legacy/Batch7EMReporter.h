// 文件目标：
// - 声明批次7路径级电磁主链验证输出函数。
//
// 主要功能：
// - 对 LOS / 单反射 / 单透射 / 单绕射路径执行模块5主链求值；
// - 输出路径级 EMPathResult 摘要；
// - 为批次7闭环提供统一日志出口。

#pragma once

#include "../../core/common/log/Logger.h"
#include "../../core/common/config/AppConfig.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 执行批次7路径级电磁主链自检并输出摘要。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">已闭环的静态场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次7自检通过；false 表示失败。</returns>
bool ReportBatch7EMSummary(const AppConfig& config, const Scene& scene, Logger& logger);

} // namespace rt
