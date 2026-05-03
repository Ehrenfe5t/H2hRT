// 文件目标：
// - 定义模块6批次9导出流程使用的统一上下文对象。
//
// 主要功能：
// - 汇总模块4/5产物、配置与输出目录信息；
// - 避免模块6各导出子过程重复散落参数列表；
// - 为 ExportBundle / Validation / Regression 流程提供统一输入。

#pragma once

#include "ExportBundle.h"
#include "RegressionReport.h"
#include "ValidationReport.h"
#include "../common/config/AppConfig.h"
#include "../em/EMProfile.h"

namespace rt {

/// <summary>
/// 模块6结果导出上下文。
/// </summary>
struct ResultExportContext {
    const AppConfig* config = nullptr;
    const EMAggregateResult* precise_result = nullptr;
    const EMAggregateResult* coverage_result = nullptr;
    std::string export_root_directory;
};

} // namespace rt
