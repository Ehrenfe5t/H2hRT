// 文件目标：
// - 实现应用层启动流水线，并把模块1、模块2、模块4/5/6批次闭环与A1真实主链连接起来。
//
// 主要功能：
// - 加载 AppConfig；
// - 初始化 Logger；
// - 输出 VersionInfo；
// - 执行模块1配置校验与自检；
// - 执行模块2批次2的场景导入与语义恢复闭环；
// - 执行模块2批次3的拓扑、诊断与加速结构闭环；
// - 在保留 Batch5~9 自检链的同时，扶正 A1 Search -> EM -> Export 真实生产链；
// - 返回统一结构化执行结果。

#include "RtPipeline.h"

#include "Batch5SearchReporter.h"
#include "Batch6ExpanderReporter.h"
#include "Batch7EMReporter.h"
#include "Batch8AggregateReporter.h"
#include "Batch9ExportReporter.h"
#include "A1RealChainRunner.h"
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
#include "../core/search/SearchEngine.h"
#include "../core/path/PathSearchContext.h"
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

/// <summary>
/// 根据配置构建批次5的最小搜索上下文。
/// </summary>
/// <param name="config">统一应用配置对象。</param>
/// <param name="scene">静态场景对象。</param>
/// <returns>批次5最小搜索上下文。</returns>
PathSearchContext BuildBatch5SearchContext(const AppConfig& config, const Scene& scene)
{
    PathSearchContext context;
    context.config = &config;
    context.scene = &scene;
    context.scene_query = scene.query.get();
    context.tx_point.x = config.path_search.debug_tx_x;
    context.tx_point.y = config.path_search.debug_tx_y;
    context.tx_point.z = config.path_search.debug_tx_z;
    context.rx_point.x = config.path_search.debug_rx_x;
    context.rx_point.y = config.path_search.debug_rx_y;
    context.rx_point.z = config.path_search.debug_rx_z;
    return context;
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

    const PathSearchContext batch5SearchContext = BuildBatch5SearchContext(loadResult.config, batch4Result.scene);
    SearchEngine searchEngine;
    const SearchEngineResult batch5Result = searchEngine.Run(batch5SearchContext);
    ReportBatch5SearchSummary(batch5Result, logger);

    if (!batch5Result.succeeded || batch5Result.path_set.paths.empty())
    {
        logger.Log(LogLevel::Fatal, "App", "Batch5 SearchEngine skeleton failed to establish a basic LOS closed loop.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "Batch5 module4 SearchEngine skeleton closed loop completed.");

    const bool batch6SelfCheckPassed = ReportBatch6ExpanderSummary(batch5SearchContext, logger);
    if (!batch6SelfCheckPassed)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch6 expanders failed to establish the required single-step closure checks.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "Batch6 module4 expanders closed loop completed.");

    const bool batch7SelfCheckPassed = ReportBatch7EMSummary(loadResult.config, batch4Result.scene, logger);
    if (!batch7SelfCheckPassed)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch7 EM main-chain self-check failed.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "Batch7 module5 EM main-chain closed loop completed.");

    const bool batch8SelfCheckPassed = ReportBatch8AggregateSummary(loadResult.config, batch4Result.scene, logger);
    if (!batch8SelfCheckPassed)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch8 EM aggregate/profile self-check failed.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "Batch8 module5 aggregate and dual-profile closed loop completed.");

    const bool batch9SelfCheckPassed = ReportBatch9ExportSummary(loadResult.config, batch4Result.scene, logger);
    if (!batch9SelfCheckPassed)
    {
        logger.Log(LogLevel::Fatal, "App", "Batch9 export/validation/regression self-check failed.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "Batch9 module6 export, validation and regression closed loop completed.");

    const A1RealChainRunResult a1Result = RunA1RealChain(loadResult.config, batch4Result.scene, batch5Result, logger);
    if (!a1Result.succeeded)
    {
        logger.Log(LogLevel::Fatal, "App", "A1 real production chain failed to replace reference path as the primary result chain.");
        return runResult;
    }

    logger.Log(LogLevel::Info, "App", "A1 real production chain closed loop completed.");

    runResult.succeeded = true;
    runResult.exit_code = 0;
    runResult.completed_batch = 10;
    return runResult;
}

} // namespace rt
