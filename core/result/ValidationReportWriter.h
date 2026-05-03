// 文件目标：
// - 声明模块6批次9的验证报告构建与导出接口。
//
// 主要功能：
// - 根据批次9导出流程构建 ValidationReport；
// - 将验证报告导出为 JSON 文本；
// - 为人工核查和自动脚本提供统一报告文件。

#pragma once

#include "ResultExportContext.h"

namespace rt {

/// <summary>
/// 构建验证报告。
/// </summary>
/// <param name="bundle">当前导出总容器。</param>
/// <param name="context">结果导出上下文。</param>
/// <returns>结构化验证报告。</returns>
ValidationReport BuildValidationReport(const ExportBundle& bundle, const ResultExportContext& context);

/// <summary>
/// 导出验证报告文件。
/// </summary>
/// <param name="report">验证报告对象。</param>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportValidationReport(const ValidationReport& report, const ResultExportContext& context, ExportBundle& bundle);

} // namespace rt
