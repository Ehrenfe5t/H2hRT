// main.cpp
// 程序入口：读取配置文件路径，委托 RtPipeline 执行 v11 P2P 主链，并返回统一退出码。

#include "RtPipeline.h"

#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

/// Program entry. argv[1] is an optional config path; default is config.json.
int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    try
    {
        // Prefer the command-line config path; otherwise use the v11 baseline config.
        const std::string configPath = (argc > 1) ? argv[1] : "config.json";

        // Run the v11 P2P main chain.
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
