// 文件目标：
// - 实现配置自检逻辑。
//
// 主要功能：
// - 载入预定义错误配置样例；
// - 验证这些错误样例确实会被模块1校验器拦截；
// - 若关键规则回退，则把问题转化为显式的流水线失败。

#include "ConfigSelfCheck.h"

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
    ConfigSelfCheckResult& result)
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

ConfigSelfCheckResult RunModule1SelfCheck(const AppConfig& config)
{
    ConfigSelfCheckResult result;

    // v9 step3: 恢复负例自检 — 内联构造错误配置验证validator能正确拦截
    struct NegTest {
        const char* name;
        const char* expectedToken;
        AppConfig badConfig;
    };

    // 以实际config为模板，逐项修改出已知错误
    AppConfig base = config;

    const NegTest tests[] = {
        {"frequency_zero", "frequency_hz",
         [&](){ auto c=base; c.em_solver.frequency_hz=0.0; return c; }()},
        {"depth_zero", "max_path_depth",
         [&](){ auto c=base; c.path_search.max_path_depth=0; return c; }()},
        {"neg_reflection", "max_reflection_count",
         [&](){ auto c=base; c.path_search.max_reflection_count=-1; return c; }()},
        {"neg_eps_length", "eps_length",
         [&](){ auto c=base; c.numeric_tolerance.eps_length=0.0; return c; }()},
        {"neg_eps_intersection", "eps_intersection",
         [&](){ auto c=base; c.numeric_tolerance.eps_intersection=0.0; return c; }()},
        {"empty_source", "source_file",
         [&](){ auto c=base; c.scene_import.source_file=""; return c; }()},
        {"empty_material_map", "scene_material_map_file",
         [&](){ auto c=base; c.scene_import.scene_material_map_file=""; return c; }()},
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0, failed = 0;

    for (int i = 0; i < numTests; ++i) {
        const auto& t = tests[i];
        ConfigValidationResult vr = ValidateAppConfig(t.badConfig);
        if (vr.passed) {
            // 负例竟然通过了 → 自检失败
            result.error = RtError::Create(
                ErrorCode::SelfCheckFailed, "Module1",
                std::string("NegTest '") + t.name + "' unexpectedly passed validation.",
                t.expectedToken,
                "Validator coverage may have regressed.", true);
            result.details.push_back(std::string("[FAIL] ") + t.name + " — should have been rejected but passed");
            failed++;
        } else {
            bool found = false;
            for (const auto& err : vr.errors)
                if (err.find(t.expectedToken) != std::string::npos) { found = true; break; }
            if (found) {
                result.details.push_back(std::string("[PASS] ") + t.name + " — correctly rejected");
                passed++;
            } else {
                result.details.push_back(std::string("[WARN] ") + t.name + " — rejected but missing expected token '" + t.expectedToken + "'");
                failed++;
            }
        }
    }

    if (failed > 0) {
        result.succeeded = false;
    } else {
        result.succeeded = true;
        result.details.push_back("All " + std::to_string(passed) + " negative cases correctly rejected.");
    }

    return result;
}

} // namespace rt
