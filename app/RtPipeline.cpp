// 文件目标：
// - 实现应用层启动流水线，把 main 与模块1基础设施连接起来。
//
// 主要功能：
// - 加载 AppConfig；
// - 初始化 Logger；
// - 输出 VersionInfo；
// - 执行配置校验；
// - 导出配置快照；
// - 触发模块1自检；
// - 形成批次0/1可编译、可运行、可验证的正式闭环。

#include "RtPipeline.h"

#include "../core/common/config/AppConfig.h"
#include "../core/common/config/AppConfigLoader.h"
#include "../core/common/config/AppConfigSnapshotWriter.h"
#include "../core/common/config/AppConfigValidator.h"
#include "../core/common/config/Module1SelfCheck.h"
#include "../core/common/log/Logger.h"
#include "../core/common/version/VersionInfo.h"

#include <sstream>

namespace rt {

namespace {

/// <summary>
/// 将配置校验结果统一写入日志系统。
/// </summary>
/// <param name="logger">已经完成初始化的统一日志对象。</param>
/// <param name="validation">待输出的配置校验结果。</param>
/// <returns>无返回值。</returns>
void LogValidationResult(Logger& logger, const ConfigValidationResult& validation)
{
    for (const std::string& warning : validation.warnings)
    {
        logger.Log(LogLevel::Warn, "Module1", warning);
    }

    for (const std::string& error : validation.errors)
    {
        logger.Log(LogLevel::Error, "Module1", error);
    }
}

} // namespace

/// <summary>
/// 执行当前 RT 应用启动主流程。
/// </summary>
/// <param name="configPath">JSON 配置文件路径。</param>
/// <returns>结构化流水线执行结果，包含成功标志和退出码。</returns>
PipelineRunResult RtPipeline::Run(const std::string& configPath) const
{
    PipelineRunResult runResult;

    // 先加载配置。即使加载失败，也尽量保留默认配置对象，
    // 这样 Logger 仍然可以基于默认 runtime 配置启动，输出统一诊断信息。
    const AppConfigLoadResult loadResult = LoadAppConfigFromJsonFile(configPath);

    Logger logger;
    logger.Initialize(loadResult.config.app_runtime);

    const VersionInfo versionInfo = VersionInfo::Current();
    logger.Log(LogLevel::Info, "App", "RT pipeline bootstrap started.");
    logger.Log(LogLevel::Info, "App", "Using config file: " + configPath);
    logger.Log(LogLevel::Info, "App", "Program version: " + versionInfo.program_version);
    logger.Log(LogLevel::Info, "App", "Config schema version: " + versionInfo.config_schema_version);
    logger.Log(LogLevel::Info, "App", "Resolved run_id: " + loadResult.config.app_runtime.run_id);

    for (const RtError& error : loadResult.errors)
    {
        logger.LogError("Module1", error);
    }

    if (!loadResult.load_succeeded)
    {
        // 若配置连基础加载都失败，则禁止进入后续模块，防止隐式 fallback。
        logger.Log(LogLevel::Fatal, "App", "Configuration load failed before validation completed.");
        return runResult;
    }

    const ConfigValidationResult validation = ValidateAppConfig(loadResult.config);
    LogValidationResult(logger, validation);

    if (!validation.passed)
    {
        // 配置校验失败必须在模块1前置阻断，避免错误参数污染后续模块。
        logger.Log(LogLevel::Fatal, "App", "Configuration validation failed.");
        return runResult;
    }

    std::ostringstream summary;
    summary << "Validation passed. Mode=" << loadResult.config.app_runtime.mode
            << ", LogLevel=" << loadResult.config.app_runtime.log_level
            << ", FrequencyHz=" << loadResult.config.em_solver.frequency_hz;
    logger.Log(LogLevel::Info, "App", summary.str());

    if (loadResult.config.output.export_config_snapshot)
    {
        const AppConfigSnapshotWriteResult snapshotWriteResult = WriteAppConfigSnapshot(loadResult.config);
        if (!snapshotWriteResult.succeeded)
        {
            logger.LogError("Module1", snapshotWriteResult.error);
            logger.Log(LogLevel::Fatal, "App", "Configuration snapshot export failed.");
            return runResult;
        }

        logger.Log(LogLevel::Info, "App", "Config snapshot exported to: " + snapshotWriteResult.output_file_path);
    }

    if (loadResult.config.validation.run_module1_self_check)
    {
        const Module1SelfCheckResult selfCheckResult = RunModule1SelfCheck(loadResult.config);
        for (const std::string& detail : selfCheckResult.details)
        {
            logger.Log(LogLevel::Info, "Module1", "SelfCheck: " + detail);
        }

        if (!selfCheckResult.succeeded)
        {
            logger.LogError("Module1", selfCheckResult.error);
            logger.Log(LogLevel::Fatal, "App", "Module1 self-check failed.");
            return runResult;
        }

        logger.Log(LogLevel::Info, "Module1", "Module1 self-check passed.");
    }

    logger.Log(LogLevel::Info, "App", "Batch0/1 bootstrap closed loop completed.");

    runResult.succeeded = true;
    runResult.exit_code = 0;
    return runResult;
}

} // namespace rt
