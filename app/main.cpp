// 文件目标：
// - 作为 RT 新系统当前阶段的控制台程序入口。
// - 负责解析命令行中的配置路径，并把控制权交给 RtPipeline。
//
// 主要功能：
// - 提供稳定默认配置路径；
// - 捕获启动阶段未处理异常；
// - 返回统一退出码，便于脚本和 VS2022 调试使用。

#include "RtPipeline.h"

#include <exception>
#include <iostream>
#include <string>

/// <summary>
/// 应用程序入口函数。
/// </summary>
/// <param name="argc">命令行参数数量。</param>
/// <param name="argv">命令行参数数组；若存在 `argv[1]`，则把它视为配置文件路径。</param>
/// <returns>返回进程退出码；0 表示成功，非 0 表示失败。</returns>
int main(int argc, char* argv[])
{
    try
    {
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
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        return -1;
    }
}
