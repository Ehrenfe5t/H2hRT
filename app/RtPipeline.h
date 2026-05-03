// 文件目标：
// - 定义应用层 RT 流水线入口对象，承接 main 与模块1基础设施之间的调用边界。
//
// 主要功能：
// - 组织配置加载、日志初始化、版本输出、配置校验等启动阶段动作；
// - 形成当前批次 0/1 以及模块2批次2/3/4的启动闭环；
// - 为后续模块2~模块6接入保留稳定的 app 层主线入口。

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 描述一次应用层流水线执行结果。
/// </summary>
struct PipelineRunResult {
    /// <summary>表示当前流水线是否成功完成启动闭环。</summary>
    bool succeeded = false;

    /// <summary>进程建议退出码。0 表示成功，非 0 表示失败。</summary>
    int exit_code = 1;

    /// <summary>表示当前运行完成到了哪个批次。</summary>
    int completed_batch = 1;
};

/// <summary>
/// RT 应用主流水线入口。
/// </summary>
class RtPipeline {
public:
    /// <summary>
    /// 根据指定配置文件执行当前批次的应用启动流程。
    /// </summary>
    /// <param name="configPath">JSON 配置文件路径。</param>
    /// <returns>结构化的流水线执行结果，包含成功标志与退出码。</returns>
    PipelineRunResult Run(const std::string& configPath) const;
};

} // namespace rt
