// ───────────────────────────────────────────────────────────────────
// 文件: main.cpp
// 用途: RT射线追踪管线控制台入口。解析命令行配置路径，委托RtPipeline执行，
//       返回统一退出码供脚本/CI使用。
// 所属模块: 应用层入口
// ───────────────────────────────────────────────────────────────────

#include "RtPipeline.h"

#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

/// <summary>
/// 程序入口。从 argv[1] 解析可选配置文件路径，委托 RtPipeline::Run，
/// 返回统一退出码（0=成功，非0=失败）。
/// </summary>
int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // v7: 修复控制台中文乱码
#endif
    try
    {
        // 解析配置路径: 命令行参数优先, 否则回退到默认配置
        const std::string configPath = (argc > 1) ? argv[1] : "configs/app/meeting_412_v9_full.json";

        // 启动完整管线 (配置→校验→批次→A1生产链→SBR覆盖)
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
