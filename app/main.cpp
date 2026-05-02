// 文件目标：
// - 作为 RT 新系统当前批次的控制台程序入口。
// - 负责解析命令行中的配置路径，并把启动控制权交给 RtPipeline。
//
// 主要功能：
// - 提供稳定的默认配置文件路径，支撑批次0/1闭环验证；
// - 捕获启动阶段未处理异常，避免程序无诊断退出；
// - 返回统一退出码，便于脚本调用、VS2022 调试和后续批处理使用。

#include "RtPipeline.h"

#include <exception>
#include <iostream>
#include <string>

/// <summary>
/// 应用程序入口函数。
/// </summary>
/// <param name="argc">命令行参数数量。</param>
/// <param name="argv">命令行参数数组；当存在 `argv[1]` 时，将其视为 JSON 配置文件路径。</param>
/// <returns>返回进程退出码；0 表示成功，非 0 表示启动失败。</returns>
int main(int argc, char* argv[])
{
    try
    {
        // 保留稳定默认配置入口，保证当前批次无需额外参数也可完成闭环验证。
        const std::string configPath = (argc > 1) ? argv[1] : "configs/app/minimal.json";

        rt::RtPipeline pipeline;
        const rt::PipelineRunResult result = pipeline.Run(configPath);

        if (!result.succeeded)
        {
            std::cerr << "RT pipeline failed." << std::endl;
        }

        return result.exit_code;
    }
    catch (const std::exception& ex)
    {
        // 即便 Logger 尚未初始化，也要保证终端可看到最基础异常信息。
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return -1;
    }
}
