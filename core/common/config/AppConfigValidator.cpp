// 文件目标：
// - 实现模块1统一配置校验器。
//
// 主要功能：
// - 执行字段级校验；
// - 执行枚举与数值范围校验；
// - 执行跨配置块依赖校验；
// - 对 transmission 等关键链路显式阻断静默 fallback。

#include "AppConfigValidator.h"

#include <fstream>

namespace rt {

namespace {

/// <summary>
/// 判断文件是否存在且可打开。
/// </summary>
/// <param name="filePath">待检查文件路径。</param>
/// <returns>若文件存在且可访问则返回 true，否则返回 false。</returns>
bool FileExists(const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }

    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    return input.good();
}

/// <summary>判断日志级别字符串是否属于受支持集合。</summary>
bool IsSupportedLogLevel(const std::string& level)
{
    return level == "TRACE" || level == "DEBUG" || level == "INFO" ||
           level == "WARN" || level == "ERROR" || level == "FATAL";
}

/// <summary>判断运行模式字符串是否受支持。</summary>
bool IsSupportedRunMode(const std::string& mode)
{
    return mode == "debug" || mode == "production";
}

/// <summary>判断场景输入格式是否受支持。</summary>
bool IsSupportedSceneFormat(const std::string& format)
{
    return format == "obj" || format == "stl" || format == "cache";
}

/// <summary>判断材料频率查询模式是否受支持。</summary>
bool IsSupportedFrequencyQueryMode(const std::string& mode)
{
    return mode == "exact" || mode == "interpolate";
}

/// <summary>判断电磁求解模式是否受支持。</summary>
bool IsSupportedSolverMode(const std::string& mode)
{
    return mode == "Precise" || mode == "Coverage";
}

} // namespace

/// <summary>
/// 校验统一应用配置对象。
/// </summary>
/// <param name="config">待校验的配置对象。</param>
/// <returns>包含 warnings 和 errors 的结构化配置校验结果。</returns>
ConfigValidationResult ValidateAppConfig(const AppConfig& config)
{
    ConfigValidationResult result;

    if (!IsSupportedRunMode(config.app_runtime.mode))
    {
        result.passed = false;
        result.errors.push_back("app_runtime.mode must be 'debug' or 'production'.");
    }

    if (!IsSupportedLogLevel(config.app_runtime.log_level))
    {
        result.passed = false;
        result.errors.push_back("app_runtime.log_level must be one of TRACE/DEBUG/INFO/WARN/ERROR/FATAL.");
    }

    if (config.app_runtime.worker_threads <= 0)
    {
        result.passed = false;
        result.errors.push_back("app_runtime.worker_threads must be > 0.");
    }

    if (config.app_runtime.config_snapshot_directory.empty())
    {
        result.passed = false;
        result.errors.push_back("app_runtime.config_snapshot_directory must not be empty.");
    }

    if (config.app_runtime.cache_directory.empty())
    {
        result.passed = false;
        result.errors.push_back("app_runtime.cache_directory must not be empty.");
    }

    if (!IsSupportedSceneFormat(config.scene_import.source_format))
    {
        result.passed = false;
        result.errors.push_back("scene_import.source_format must be obj/stl/cache.");
    }

    if (config.scene_import.source_file.empty())
    {
        result.passed = false;
        result.errors.push_back("scene_import.source_file must not be empty.");
    }
    else if (!FileExists(config.scene_import.source_file))
    {
        result.passed = false;
        result.errors.push_back("scene_import.source_file does not exist: " + config.scene_import.source_file);
    }

    if (config.scene_preprocess.bvh_leaf_size <= 0)
    {
        result.passed = false;
        result.errors.push_back("scene_preprocess.bvh_leaf_size must be > 0.");
    }

    if (config.scene_preprocess.wedge_min_angle_deg <= 0.0 ||
        config.scene_preprocess.wedge_max_angle_deg <= 0.0 ||
        config.scene_preprocess.wedge_max_angle_deg <= config.scene_preprocess.wedge_min_angle_deg)
    {
        result.passed = false;
        result.errors.push_back("scene_preprocess wedge angle range is invalid.");
    }

    if (!IsSupportedFrequencyQueryMode(config.material.frequency_query_mode))
    {
        result.passed = false;
        result.errors.push_back("material.frequency_query_mode must be exact or interpolate.");
    }

    if (config.material.material_database_file.empty())
    {
        result.passed = false;
        result.errors.push_back("material.material_database_file must not be empty.");
    }
    else if (!FileExists(config.material.material_database_file))
    {
        result.passed = false;
        result.errors.push_back("material.material_database_file does not exist: " + config.material.material_database_file);
    }

    if (config.path_search.max_path_depth < 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_path_depth must be >= 0.");
    }

    if (config.path_search.max_reflection_count < 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_reflection_count must be >= 0.");
    }

    if (config.path_search.max_transmission_count < 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_transmission_count must be >= 0.");
    }

    if (config.path_search.max_diffraction_count < 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_diffraction_count must be >= 0.");
    }

    if (config.path_search.max_scattering_count < 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_scattering_count must be >= 0.");
    }

    if (config.path_search.max_candidate_face_hits <= 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_candidate_face_hits must be > 0.");
    }

    if (config.path_search.max_candidate_wedges <= 0)
    {
        result.passed = false;
        result.errors.push_back("path_search.max_candidate_wedges must be > 0.");
    }

    if (!config.path_search.enable_los && !config.path_search.enable_reflection &&
        !config.path_search.enable_transmission && !config.path_search.enable_diffraction &&
        !config.path_search.enable_scattering)
    {
        result.passed = false;
        result.errors.push_back("At least one path_search mechanism must be enabled.");
    }

    if (config.path_search.max_reflection_count + config.path_search.max_transmission_count +
        config.path_search.max_diffraction_count + config.path_search.max_scattering_count > config.path_search.max_path_depth &&
        config.path_search.max_path_depth > 0)
    {
        result.warnings.push_back("The sum of interaction-specific budgets exceeds max_path_depth; runtime search will still be limited by max_path_depth.");
    }

    if (!IsSupportedSolverMode(config.em_solver.solver_mode))
    {
        result.passed = false;
        result.errors.push_back("em_solver.solver_mode must be Precise or Coverage.");
    }

    if (config.em_solver.frequency_hz <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("em_solver.frequency_hz must be > 0.");
    }

    if (config.output.output_directory.empty())
    {
        result.passed = false;
        result.errors.push_back("output.output_directory must not be empty.");
    }

    if (config.output.export_aps && !config.path_search.keep_angle_metadata)
    {
        result.passed = false;
        result.errors.push_back("output.export_aps requires path_search.keep_angle_metadata = true.");
    }

    if (config.validation.run_module1_self_check)
    {
        if (config.validation.module1_invalid_transmission_case_file.empty())
        {
            result.passed = false;
            result.errors.push_back("validation.module1_invalid_transmission_case_file must not be empty when run_module1_self_check is enabled.");
        }
        else if (!FileExists(config.validation.module1_invalid_transmission_case_file))
        {
            result.passed = false;
            result.errors.push_back("validation.module1_invalid_transmission_case_file does not exist: " + config.validation.module1_invalid_transmission_case_file);
        }

        if (config.validation.module1_invalid_diffraction_case_file.empty())
        {
            result.passed = false;
            result.errors.push_back("validation.module1_invalid_diffraction_case_file must not be empty when run_module1_self_check is enabled.");
        }
        else if (!FileExists(config.validation.module1_invalid_diffraction_case_file))
        {
            result.passed = false;
            result.errors.push_back("validation.module1_invalid_diffraction_case_file does not exist: " + config.validation.module1_invalid_diffraction_case_file);
        }
    }

    if (config.path_search.enable_transmission)
    {
        if (config.material.material_mapping_file.empty())
        {
            result.passed = false;
            result.errors.push_back("Transmission is enabled, so material.material_mapping_file must not be empty.");
        }
        else if (!FileExists(config.material.material_mapping_file))
        {
            result.passed = false;
            result.errors.push_back("Transmission is enabled, but material.material_mapping_file does not exist: " + config.material.material_mapping_file);
        }

        if (config.material.allow_material_fallback)
        {
            result.passed = false;
            result.errors.push_back("Transmission path is enabled, but material.allow_material_fallback must remain false to avoid silent fallback.");
        }
    }
    else if (config.path_search.max_transmission_count > 0)
    {
        result.warnings.push_back("max_transmission_count is positive while transmission is disabled; the budget will be ignored.");
    }

    if (config.path_search.enable_diffraction)
    {
        if (!config.scene_preprocess.enable_wedge_build)
        {
            result.passed = false;
            result.errors.push_back("Diffraction is enabled, but scene_preprocess.enable_wedge_build is false.");
        }
    }
    else if (config.path_search.max_diffraction_count > 0)
    {
        result.warnings.push_back("max_diffraction_count is positive while diffraction is disabled; the budget will be ignored.");
    }

    if (config.path_search.enable_reflection && config.path_search.max_reflection_count == 0)
    {
        result.warnings.push_back("Reflection is enabled but max_reflection_count is 0; reflection paths will not be expanded.");
    }

    if (config.path_search.enable_transmission && config.path_search.max_transmission_count == 0)
    {
        result.warnings.push_back("Transmission is enabled but max_transmission_count is 0; transmission paths will not be expanded.");
    }

    if (config.path_search.enable_diffraction && config.path_search.max_diffraction_count == 0)
    {
        result.warnings.push_back("Diffraction is enabled but max_diffraction_count is 0; diffraction paths will not be expanded.");
    }

    if (config.antenna.source_type != "Ideal")
    {
        if (config.antenna.pattern_file.empty() && config.antenna.polarization_file.empty())
        {
            result.passed = false;
            result.errors.push_back("Non-Ideal antenna source requires at least pattern_file or polarization_file.");
        }
    }

    const ConfigValidationResult toleranceValidation = config.numeric_tolerance.Validate();
    for (const std::string& warning : toleranceValidation.warnings)
    {
        result.warnings.push_back(warning);
    }

    for (const std::string& error : toleranceValidation.errors)
    {
        result.passed = false;
        result.errors.push_back(error);
    }

    if (config.app_runtime.mode == "production" && config.output.export_debug_files)
    {
        result.warnings.push_back("production mode currently keeps export_debug_files enabled; consider disabling for later batches.");
    }

    return result;
}

} // namespace rt
