// 文件目标：
// - 实现批次5 SearchEngine 骨架验证输出函数。
//
// 主要功能：
// - 输出状态级统计、路径级统计与 trace 摘要；
// - 为人工核查 LOS 闭环是否成立提供统一日志入口；
// - 控制 trace 输出数量，避免日志噪声过大。

#include "Batch5SearchReporter.h"

#include <sstream>

namespace rt {

/// <summary>
/// 将批次5 SearchEngine 摘要输出到日志系统。
/// </summary>
/// <param name="result">搜索执行结果。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportBatch5SearchSummary(const SearchEngineResult& result, Logger& logger)
{
    std::ostringstream summary;
    summary << "Batch5Search: succeeded=" << (result.succeeded ? "true" : "false")
            << ", states=" << result.generated_state_count
            << ", dedup_states=" << result.deduplicated_state_count
            << ", dedup_paths=" << result.deduplicated_path_count
            << ", paths=" << result.path_set.paths.size();
    logger.Log(LogLevel::Info, "Module4", summary.str());

    for (const auto& item : result.failure_reason_counts)
    {
        std::ostringstream failureStream;
        failureStream << "ExpanderFailureReason: code=" << item.first << ", count=" << item.second;
        logger.Log(LogLevel::Info, "Module4", failureStream.str());
    }

    int traceCount = 0;
    for (const std::string& traceLine : result.trace_lines)
    {
        if (traceCount >= 16)
        {
            break;
        }
        logger.Log(LogLevel::Info, "Module4", "SearchTrace: " + traceLine);
        ++traceCount;
    }
}

} // namespace rt
