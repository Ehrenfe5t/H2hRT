// ───────────────────────────────────────────────────────────────────
// 文件: RtPipeline.h
// 用途: 应用层管线入口声明。持有完整的启动序列: 配置加载→校验→批次闭环→
//       A1真实生产链→可选SBR覆盖。
// 所属模块: 应用层
// ───────────────────────────────────────────────────────────────────

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 单次管线调用的结构化返回结果。
/// </summary>
struct PipelineRunResult {
    /// <summary>管线闭环是否成功完成。</summary>
    bool succeeded = false;

    /// <summary>建议进程退出码; 0=成功, 非0=失败。</summary>
    int exit_code = 1;

    /// <summary>最后完成的批次号 (1-indexed)。</summary>
    int completed_batch = 1;
};

/// <summary>
/// 顶层管线入口。持有配置加载、校验、全部批次闭环、A1真实链及可选SBR通道。
/// </summary>
class RtPipeline {
public:
    /// <summary>
    /// 从JSON配置文件执行完整的应用启动管线。
    /// </summary>
    /// <param name="configPath">JSON配置文件路径。</param>
    /// <returns>包含成功标志、退出码和已完成批次的结构化结果。</returns>
    PipelineRunResult Run(const std::string& configPath) const;
};

} // namespace rt
