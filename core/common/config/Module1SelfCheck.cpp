// 文件目标：
// - 实现模块1自检逻辑。
//
// 主要功能：
// - 载入预定义错误配置样例；
// - 验证这些错误样例确实会被模块1校验器拦截；
// - 若关键规则回退，则把问题转化为显式的流水线失败。

#include "Module1SelfCheck.h"

#include "AppConfigLoader.h"
#include "AppConfigValidator.h"

namespace rt {

namespace {

/// <summary>
/// 判断校验结果中是否包含指定错误关键词。
/// </summary>
/// <param name="validation">待检查的配置校验结果。</param>
/// <param name="token">期望出现的错误关键词。</param>
/// <returns>若任一错误信息中包含该关键词则返回 true，否则返回 false。</returns>
bool ContainsErrorToken(const ConfigValidationResult& validation, const std::string& token)
{
    for (const std::string& error : validation.errors)
    {
        if (error.find(token) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

/// <summary>
/// 执行一个负样例自检用例。
/// </summary>
/// <param name="filePath">负样例配置文件路径。</param>
/// <param name="expectedToken">期望命中的错误关键词。</param>
/// <param name="caseName">当前用例的人类可读名称。</param>
/// <param name="result">累计写入的自检结果对象。</param>
/// <returns>若该负样例行为符合预期则返回 true，否则返回 false。</returns>
bool RunNegativeCase(
    const std::string& filePath,
    const std::string& expectedToken,
    const std::string& caseName,
    Module1SelfCheckResult& result)
{
    const AppConfigLoadResult loadResult = LoadAppConfigFromJsonFile(filePath);
    if (!loadResult.load_succeeded)
    {
        result.error = RtError::Create(
            ErrorCode::SelfCheckFailed,
            "Module1",
            "模块1自检在加载负样例配置文件时失败。",
            filePath,
            "请检查自检配置文件是否存在且 JSON 格式正确。",
            true);
        return false;
    }

    const ConfigValidationResult validation = ValidateAppConfig(loadResult.config);
    if (validation.passed)
    {
        result.error = RtError::Create(
            ErrorCode::SelfCheckFailed,
            "Module1",
            "模块1自检负样例意外通过了配置校验。",
            caseName,
            "请检查该关键规则是否从校验器中回退。",
            true);
        return false;
    }

    if (!ContainsErrorToken(validation, expectedToken))
    {
        result.error = RtError::Create(
            ErrorCode::SelfCheckFailed,
            "Module1",
            "模块1自检负样例虽然失败，但失败原因集合不符合预期。",
            caseName,
            "请检查校验器输出内容与期望规则是否一致。",
            true);
        return false;
    }

    result.details.push_back(caseName + " passed.");
    return true;
}

} // namespace

/// <summary>
/// 执行模块1自检用例集合。
/// </summary>
/// <param name="config">当前已通过主校验的应用配置对象。</param>
/// <returns>结构化的模块1自检结果。</returns>
Module1SelfCheckResult RunModule1SelfCheck(const AppConfig& config)
{
    Module1SelfCheckResult result;

    if (!RunNegativeCase(
            config.validation.module1_invalid_transmission_case_file,
            "material.material_mapping_file",
            "NegativeCase.TransmissionMissingMapping",
            result))
    {
        return result;
    }

    if (!RunNegativeCase(
            config.validation.module1_invalid_diffraction_case_file,
            "enable_wedge_build",
            "NegativeCase.DiffractionWithoutWedge",
            result))
    {
        return result;
    }

    result.succeeded = true;
    return result;
}

} // namespace rt
