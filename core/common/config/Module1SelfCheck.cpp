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
            "Module1 self-check failed while loading a negative case file.",
            filePath,
            "Check the self-check config file and JSON format.",
            true);
        return false;
    }

    const ConfigValidationResult validation = ValidateAppConfig(loadResult.config);
    if (validation.passed)
    {
        result.error = RtError::Create(
            ErrorCode::SelfCheckFailed,
            "Module1",
            "Module1 self-check negative case unexpectedly passed validation.",
            caseName,
            "Review validator coverage for this critical startup rule.",
            true);
        return false;
    }

    if (!ContainsErrorToken(validation, expectedToken))
    {
        result.error = RtError::Create(
            ErrorCode::SelfCheckFailed,
            "Module1",
            "Module1 self-check negative case failed for an unexpected reason set.",
            caseName,
            "Review validator error content and expected rule coverage.",
            true);
        return false;
    }

    result.details.push_back(caseName + " passed.");
    return true;
}

} // namespace

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
