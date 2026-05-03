// 文件目标：
// - 实现应用层启动流水线，并把模块1与模块2批次2/3闭环连接起来。
//
// 主要功能：
// - 加载 AppConfig；
// - 初始化 Logger；
// - 输出 VersionInfo；
// - 执行模块1配置校验与自检；
// - 执行模块2批次2的场景导入与语义恢复闭环；
// - 执行模块2批次3的拓扑、诊断与加速结构闭环；
// - 返回统一结构化执行结果。

#include "RtPipeline.h"

#include "SceneBatch2Reporter.h"
#include "SceneBatch3Reporter.h"
#include "SceneBatch4Reporter.h"
#include "../core/common/config/AppConfig.h"
#include "../core/common/config/AppConfigLoader.h"
#include "../core/common/config/AppConfigSnapshotWriter.h"
#include "../core/common/config/AppConfigValidator.h"
#include "../core/common/config/Module1SelfCheck.h"
#include "../core/common/log/Logger.h"
#include "../core/common/version/VersionInfo.h"
#include "../preprocess/build/SceneBatch2Builder.h"
#include "../preprocess/build/SceneBatch3Builder.h"
#include "../preprocess/build/SceneBatch4Builder.h"

#include <sstream>

namespace rt {

namespace {

/// <summary>
/// 将配置校验结果统一写入日志系统。
/// </summary>
/// <param name="logger">已初始化的统一日志对象。</param>
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
/// <returns>结构化流水线执行结果，包含成功标志、退出码与完成批次号。</returns>
PipelineRunResult RtPipeline::Run(const std::string& configPath) const
{
    PipelineRunResult runResult;

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
        logger.Log(LogLevel::Fatal, "App", "Configuration load failed before validation completed.");
        return runResult;
    }

    const ConfigValidationResult validation = ValidateAppConfig(loadResult.config);
    LogValidationResult(logger, validation);

    if (!validation.passed)
    {
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

    const SceneBatch2BuildResult batch2Result = BuildSceneForBatch2(loadResult.config);
    for (const RtError& error : batch2Result.errors)
    {
        logger.LogError("Module2", error);
    }

    if (!batch2Result.succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch2 scene import and semantic recovery failed.");
        return runResult;
    }

    ReportSceneBatch2Summary(batch2Result.scene, logger);
    logger.Log(LogLevel::Info, "App", "Batch2 scene import and semantic recovery closed loop completed.");

    const SceneBatch3BuildResult batch3Result = BuildSceneForBatch3(loadResult.config, batch2Result.scene);
    for (const RtError& error : batch3Result.errors)
    {
        logger.LogError("Module2", error);
    }

    if (!batch3Result.succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch3 topology, diagnostics and acceleration build failed.");
        return runResult;
    }

    ReportSceneBatch3Summary(batch3Result.scene, logger);
    logger.Log(LogLevel::Info, "App", "Batch3 topology, diagnostics and acceleration closed loop completed.");

    const SceneBatch4BuildResult batch4Result = BuildSceneForBatch4(loadResult.config, batch3Result.scene);
    for (const RtError& error : batch4Result.errors)
    {
        logger.LogError("Module2", error);
    }

    if (!batch4Result.succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch4 query facade and scene cache build failed.");
        return runResult;
    }

    ReportSceneBatch4Summary(batch4Result, logger);
    logger.Log(LogLevel::Info, "App", "Batch4 query facade and scene cache closed loop completed.");

    runResult.succeeded = true;
    runResult.exit_code = 0;
    runResult.completed_batch = 4;
    return runResult;
}

} // namespace rt
