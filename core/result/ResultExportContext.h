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
#include "../em/BuildBroadbandCFR.h"
#include "../search/SearchEngine.h"

namespace rt {

/// <summary>
/// 模块6结果导出上下文。
/// </summary>
struct ResultExportContext {
    const AppConfig* config = nullptr;
    const SearchEngineResult* search_result = nullptr;
    const EMAggregateResult* precise_result = nullptr;
    const EMAggregateResult* coverage_result = nullptr;
    const BroadbandChannelResult* broadband_result = nullptr; // v9 Stage5
    bool real_chain_enabled = false;
    std::string primary_input_source;
    std::string export_root_directory;
    std::string export_purpose = "analysis_friendly_output";
    std::string handoff_view_name = "a8_handoff_view";
};

} // namespace rt
