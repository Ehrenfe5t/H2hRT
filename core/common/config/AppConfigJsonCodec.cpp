// 文件目标：
// - 实现模块1统一 JSON 编解码层，并且不依赖额外外部 JSON 包。
//
// 主要功能：
// - 将配置读取逻辑集中到单一 codec 层，而不是分散在多个文件中；
// - 将配置快照写出逻辑与配置加载逻辑绑定到同一份 schema 映射；
// - 为后续若替换成第三方 JSON 库预留稳定接口，同时保证当前批次可维护、可闭环。

#include "AppConfigJsonCodec.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace rt {

namespace {

/// <summary>
/// 读取整个文件到内存字符串。
/// </summary>
/// <param name="filePath">输入文件路径。</param>
/// <param name="content">输出的文件全文缓冲区。</param>
/// <returns>读取成功返回 true，否则返回 false。</returns>
bool ReadWholeFile(const std::string& filePath, std::string& content)
{
    std::ifstream input(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
    {
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    content = buffer.str();
    return true;
}

/// <summary>
/// 从指定位置开始跳过空白字符。
/// </summary>
/// <param name="text">输入 JSON 文本。</param>
/// <param name="index">起始位置。</param>
/// <returns>跳过空白后的新位置。</returns>
std::size_t SkipWhitespace(const std::string& text, std::size_t index)
{
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index])) != 0)
    {
        ++index;
    }

    return index;
}

/// <summary>
/// 提取某个顶层对象对应的 JSON 对象体文本。
/// </summary>
/// <param name="text">完整 JSON 文本。</param>
/// <param name="key">目标对象键名。</param>
/// <param name="body">输出对象体文本。</param>
/// <returns>成功提取返回 true，否则返回 false。</returns>
bool ExtractObjectBody(const std::string& text, const std::string& key, std::string& body)
{
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = text.find(token);
    if (keyPos == std::string::npos)
    {
        return false;
    }

    const std::size_t colonPos = text.find(':', keyPos + token.size());
    if (colonPos == std::string::npos)
    {
        return false;
    }

    std::size_t bracePos = SkipWhitespace(text, colonPos + 1U);
    if (bracePos >= text.size() || text[bracePos] != '{')
    {
        return false;
    }

    int depth = 0;
    bool inString = false;
    for (std::size_t i = bracePos; i < text.size(); ++i)
    {
        const char ch = text[i];
        const bool escaped = (i > 0U && text[i - 1U] == '\\');

        if (ch == '"' && !escaped)
        {
            inString = !inString;
        }

        if (inString)
        {
            continue;
        }

        if (ch == '{')
        {
            ++depth;
        }
        else if (ch == '}')
        {
            --depth;
            if (depth == 0)
            {
                body = text.substr(bracePos, i - bracePos + 1U);
                return true;
            }
        }
    }

    return false;
}

/// <summary>
/// 从对象体中提取某个字段的原始值文本。
/// </summary>
/// <param name="objectBody">对象体文本。</param>
/// <param name="key">字段键名。</param>
/// <param name="rawValue">输出原始值文本。</param>
/// <returns>成功提取返回 true，否则返回 false。</returns>
bool ExtractRawValue(const std::string& objectBody, const std::string& key, std::string& rawValue)
{
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = objectBody.find(token);
    if (keyPos == std::string::npos)
    {
        return false;
    }

    const std::size_t colonPos = objectBody.find(':', keyPos + token.size());
    if (colonPos == std::string::npos)
    {
        return false;
    }

    std::size_t valuePos = SkipWhitespace(objectBody, colonPos + 1U);
    if (valuePos >= objectBody.size())
    {
        return false;
    }

    if (objectBody[valuePos] == '"')
    {
        ++valuePos;
        std::size_t end = valuePos;
        while (end < objectBody.size())
        {
            if (objectBody[end] == '"' && objectBody[end - 1U] != '\\')
            {
                rawValue = objectBody.substr(valuePos, end - valuePos);
                return true;
            }

            ++end;
        }

        return false;
    }

    std::size_t end = valuePos;
    while (end < objectBody.size())
    {
        const char ch = objectBody[end];
        if (ch == ',' || ch == '}' || std::isspace(static_cast<unsigned char>(ch)) != 0)
        {
            break;
        }

        ++end;
    }

    rawValue = objectBody.substr(valuePos, end - valuePos);
    return !rawValue.empty();
}

/// <summary>
/// 读取可选数值字段并写入目标变量。
/// </summary>
/// <typeparam name="T">目标数值类型。</typeparam>
/// <param name="objectBody">对象体文本。</param>
/// <param name="key">字段键名。</param>
/// <param name="target">目标变量。</param>
/// <returns>无返回值。</returns>
template <typename T>
void ReadOptionalNumber(const std::string& objectBody, const std::string& key, T& target)
{
    std::string value;
    if (ExtractRawValue(objectBody, key, value))
    {
        std::istringstream stream(value);
        T parsedValue{};
        stream >> parsedValue;
        if (!stream.fail())
        {
            target = parsedValue;
        }
    }
}

/// <summary>
/// 读取可选字符串字段并写入目标变量。
/// </summary>
/// <param name="objectBody">对象体文本。</param>
/// <param name="key">字段键名。</param>
/// <param name="target">目标变量。</param>
/// <returns>无返回值。</returns>
void ReadOptionalString(const std::string& objectBody, const std::string& key, std::string& target)
{
    std::string value;
    if (ExtractRawValue(objectBody, key, value))
    {
        target = value;
    }
}

/// <summary>
/// 读取可选布尔字段并写入目标变量。
/// </summary>
/// <param name="objectBody">对象体文本。</param>
/// <param name="key">字段键名。</param>
/// <param name="target">目标变量。</param>
/// <returns>无返回值。</returns>
void ReadOptionalBool(const std::string& objectBody, const std::string& key, bool& target)
{
    std::string value;
    if (ExtractRawValue(objectBody, key, value))
    {
        target = (value == "true" || value == "TRUE" || value == "True");
    }
}

/// <summary>
/// 根据 JSON 文本填充 AppConfig。
/// </summary>
/// <param name="text">完整 JSON 文本。</param>
/// <param name="config">待填充的 AppConfig 对象。</param>
/// <returns>当前实现始终返回 true；若后续扩展更严格语法检查，可在此返回 false。</returns>
bool PopulateAppConfigFromJsonText(const std::string& text, AppConfig& config)
{
    std::string body;

    if (ExtractObjectBody(text, "app_runtime", body))
    {
        ReadOptionalString(body, "mode", config.app_runtime.mode);
        ReadOptionalString(body, "log_level", config.app_runtime.log_level);
        ReadOptionalBool(body, "enable_console_logging", config.app_runtime.enable_console_logging);
        ReadOptionalBool(body, "enable_file_logging", config.app_runtime.enable_file_logging);
        ReadOptionalString(body, "log_file_path", config.app_runtime.log_file_path);
        ReadOptionalString(body, "config_snapshot_directory", config.app_runtime.config_snapshot_directory);
        ReadOptionalString(body, "cache_directory", config.app_runtime.cache_directory);
        ReadOptionalString(body, "run_id", config.app_runtime.run_id);
        ReadOptionalNumber(body, "worker_threads", config.app_runtime.worker_threads);
    }

    if (ExtractObjectBody(text, "scene_import", body))
    {
        ReadOptionalString(body, "source_file", config.scene_import.source_file);
        ReadOptionalString(body, "source_format", config.scene_import.source_format);
        ReadOptionalBool(body, "normalize_object_names", config.scene_import.normalize_object_names);
        ReadOptionalBool(body, "allow_name_auto_cleanup", config.scene_import.allow_name_auto_cleanup);
    }

    if (ExtractObjectBody(text, "scene_preprocess", body))
    {
        ReadOptionalBool(body, "rebuild_normals", config.scene_preprocess.rebuild_normals);
        ReadOptionalBool(body, "enable_wedge_build", config.scene_preprocess.enable_wedge_build);
        ReadOptionalBool(body, "enable_scene_cache", config.scene_preprocess.enable_scene_cache);
        ReadOptionalNumber(body, "wedge_min_angle_deg", config.scene_preprocess.wedge_min_angle_deg);
        ReadOptionalNumber(body, "wedge_max_angle_deg", config.scene_preprocess.wedge_max_angle_deg);
        ReadOptionalNumber(body, "bvh_leaf_size", config.scene_preprocess.bvh_leaf_size);
    }

    if (ExtractObjectBody(text, "material", body))
    {
        ReadOptionalString(body, "material_database_file", config.material.material_database_file);
        ReadOptionalString(body, "material_mapping_file", config.material.material_mapping_file);
        ReadOptionalString(body, "frequency_query_mode", config.material.frequency_query_mode);
        ReadOptionalBool(body, "allow_material_fallback", config.material.allow_material_fallback);
        ReadOptionalString(body, "default_background_medium", config.material.default_background_medium);
    }

    if (ExtractObjectBody(text, "antenna", body))
    {
        ReadOptionalString(body, "source_type", config.antenna.source_type);
        ReadOptionalString(body, "pattern_file", config.antenna.pattern_file);
        ReadOptionalString(body, "polarization_file", config.antenna.polarization_file);
    }

    if (ExtractObjectBody(text, "path_search", body))
    {
        ReadOptionalNumber(body, "max_path_depth", config.path_search.max_path_depth);
        ReadOptionalNumber(body, "max_reflection_count", config.path_search.max_reflection_count);
        ReadOptionalNumber(body, "max_transmission_count", config.path_search.max_transmission_count);
        ReadOptionalNumber(body, "max_diffraction_count", config.path_search.max_diffraction_count);
        ReadOptionalNumber(body, "max_scattering_count", config.path_search.max_scattering_count);
        ReadOptionalNumber(body, "max_candidate_face_hits", config.path_search.max_candidate_face_hits);
        ReadOptionalNumber(body, "max_candidate_wedges", config.path_search.max_candidate_wedges);
        ReadOptionalBool(body, "enable_los", config.path_search.enable_los);
        ReadOptionalBool(body, "enable_reflection", config.path_search.enable_reflection);
        ReadOptionalBool(body, "enable_transmission", config.path_search.enable_transmission);
        ReadOptionalBool(body, "enable_diffraction", config.path_search.enable_diffraction);
        ReadOptionalBool(body, "enable_scattering", config.path_search.enable_scattering);
        ReadOptionalBool(body, "enable_mixed_path", config.path_search.enable_mixed_path);
        ReadOptionalBool(body, "keep_angle_metadata", config.path_search.keep_angle_metadata);
        ReadOptionalString(body, "pruning_strategy", config.path_search.pruning_strategy);
        ReadOptionalString(body, "dedup_strategy", config.path_search.dedup_strategy);
    }

    if (ExtractObjectBody(text, "em_solver", body))
    {
        ReadOptionalNumber(body, "frequency_hz", config.em_solver.frequency_hz);
        ReadOptionalString(body, "solver_mode", config.em_solver.solver_mode);
        ReadOptionalBool(body, "enable_polarization", config.em_solver.enable_polarization);
    }

    if (ExtractObjectBody(text, "output", body))
    {
        ReadOptionalString(body, "output_directory", config.output.output_directory);
        ReadOptionalBool(body, "export_paths", config.output.export_paths);
        ReadOptionalBool(body, "export_cir", config.output.export_cir);
        ReadOptionalBool(body, "export_pdp", config.output.export_pdp);
        ReadOptionalBool(body, "export_aps", config.output.export_aps);
        ReadOptionalBool(body, "export_debug_files", config.output.export_debug_files);
        ReadOptionalBool(body, "export_config_snapshot", config.output.export_config_snapshot);
    }

    if (ExtractObjectBody(text, "validation", body))
    {
        ReadOptionalBool(body, "enable_basic_validation", config.validation.enable_basic_validation);
        ReadOptionalBool(body, "enable_reference_compare", config.validation.enable_reference_compare);
        ReadOptionalBool(body, "run_module1_self_check", config.validation.run_module1_self_check);
        ReadOptionalString(body, "module1_invalid_transmission_case_file", config.validation.module1_invalid_transmission_case_file);
        ReadOptionalString(body, "module1_invalid_diffraction_case_file", config.validation.module1_invalid_diffraction_case_file);
        ReadOptionalNumber(body, "power_tolerance_db", config.validation.power_tolerance_db);
    }

    if (ExtractObjectBody(text, "experiment", body))
    {
        ReadOptionalString(body, "experiment_tag", config.experiment.experiment_tag);
        ReadOptionalString(body, "dataset_tag", config.experiment.dataset_tag);
    }

    if (ExtractObjectBody(text, "numeric_tolerance", body))
    {
        ReadOptionalNumber(body, "eps_length", config.numeric_tolerance.eps_length);
        ReadOptionalNumber(body, "eps_angle", config.numeric_tolerance.eps_angle);
        ReadOptionalNumber(body, "eps_intersection", config.numeric_tolerance.eps_intersection);
        ReadOptionalNumber(body, "eps_normal", config.numeric_tolerance.eps_normal);
        ReadOptionalNumber(body, "eps_deduplicate", config.numeric_tolerance.eps_deduplicate);
        ReadOptionalNumber(body, "eps_power", config.numeric_tolerance.eps_power);
        ReadOptionalNumber(body, "self_hit_ignore_distance", config.numeric_tolerance.self_hit_ignore_distance);
        ReadOptionalNumber(body, "visibility_origin_offset", config.numeric_tolerance.visibility_origin_offset);
        ReadOptionalNumber(body, "visibility_target_shrink", config.numeric_tolerance.visibility_target_shrink);
    }

    return true;
}

/// <summary>
/// 将字符串转义为 JSON 安全格式。
/// </summary>
/// <param name="value">输入字符串。</param>
/// <returns>可安全写入 JSON 字符串上下文的转义结果。</returns>
std::string EscapeJsonString(const std::string& value)
{
    std::string result;
    result.reserve(value.size() + 8U);

    for (const char ch : value)
    {
        switch (ch)
        {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result.push_back(ch); break;
        }
    }

    return result;
}

} // namespace

/// <summary>
/// 从 JSON 文件解码 AppConfig。
/// </summary>
/// <param name="filePath">输入 JSON 文件路径。</param>
/// <returns>结构化解码结果，包含成功标志、配置对象和错误信息。</returns>
AppConfigJsonDecodeResult DecodeAppConfigFromJsonFile(const std::string& filePath)
{
    AppConfigJsonDecodeResult result;

    std::string content;
    if (!ReadWholeFile(filePath, content))
    {
        result.error_message = "无法读取 JSON 配置文件内容。";
        return result;
    }

    if (content.find('{') == std::string::npos)
    {
        result.error_message = "配置文件中未找到 JSON 对象根节点。";
        return result;
    }

    if (!PopulateAppConfigFromJsonText(content, result.config))
    {
        result.error_message = "将 JSON 文本映射到 AppConfig 时失败。";
        return result;
    }

    result.succeeded = true;
    return result;
}

/// <summary>
/// 将 AppConfig 编码为格式化 JSON 文本。
/// </summary>
/// <param name="config">待序列化的配置对象。</param>
/// <returns>格式化后的 JSON 字符串。</returns>
std::string EncodeAppConfigToJsonString(const AppConfig& config)
{
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"app_runtime\": {\n";
    stream << "    \"mode\": \"" << EscapeJsonString(config.app_runtime.mode) << "\",\n";
    stream << "    \"log_level\": \"" << EscapeJsonString(config.app_runtime.log_level) << "\",\n";
    stream << "    \"enable_console_logging\": " << (config.app_runtime.enable_console_logging ? "true" : "false") << ",\n";
    stream << "    \"enable_file_logging\": " << (config.app_runtime.enable_file_logging ? "true" : "false") << ",\n";
    stream << "    \"log_file_path\": \"" << EscapeJsonString(config.app_runtime.log_file_path) << "\",\n";
    stream << "    \"config_snapshot_directory\": \"" << EscapeJsonString(config.app_runtime.config_snapshot_directory) << "\",\n";
    stream << "    \"cache_directory\": \"" << EscapeJsonString(config.app_runtime.cache_directory) << "\",\n";
    stream << "    \"run_id\": \"" << EscapeJsonString(config.app_runtime.run_id) << "\",\n";
    stream << "    \"worker_threads\": " << config.app_runtime.worker_threads << "\n";
    stream << "  },\n";

    stream << "  \"scene_import\": {\n";
    stream << "    \"source_file\": \"" << EscapeJsonString(config.scene_import.source_file) << "\",\n";
    stream << "    \"source_format\": \"" << EscapeJsonString(config.scene_import.source_format) << "\",\n";
    stream << "    \"normalize_object_names\": " << (config.scene_import.normalize_object_names ? "true" : "false") << ",\n";
    stream << "    \"allow_name_auto_cleanup\": " << (config.scene_import.allow_name_auto_cleanup ? "true" : "false") << "\n";
    stream << "  },\n";

    stream << "  \"scene_preprocess\": {\n";
    stream << "    \"rebuild_normals\": " << (config.scene_preprocess.rebuild_normals ? "true" : "false") << ",\n";
    stream << "    \"enable_wedge_build\": " << (config.scene_preprocess.enable_wedge_build ? "true" : "false") << ",\n";
    stream << "    \"enable_scene_cache\": " << (config.scene_preprocess.enable_scene_cache ? "true" : "false") << ",\n";
    stream << "    \"wedge_min_angle_deg\": " << config.scene_preprocess.wedge_min_angle_deg << ",\n";
    stream << "    \"wedge_max_angle_deg\": " << config.scene_preprocess.wedge_max_angle_deg << ",\n";
    stream << "    \"bvh_leaf_size\": " << config.scene_preprocess.bvh_leaf_size << "\n";
    stream << "  },\n";

    stream << "  \"material\": {\n";
    stream << "    \"material_database_file\": \"" << EscapeJsonString(config.material.material_database_file) << "\",\n";
    stream << "    \"material_mapping_file\": \"" << EscapeJsonString(config.material.material_mapping_file) << "\",\n";
    stream << "    \"frequency_query_mode\": \"" << EscapeJsonString(config.material.frequency_query_mode) << "\",\n";
    stream << "    \"allow_material_fallback\": " << (config.material.allow_material_fallback ? "true" : "false") << ",\n";
    stream << "    \"default_background_medium\": \"" << EscapeJsonString(config.material.default_background_medium) << "\"\n";
    stream << "  },\n";

    stream << "  \"antenna\": {\n";
    stream << "    \"source_type\": \"" << EscapeJsonString(config.antenna.source_type) << "\",\n";
    stream << "    \"pattern_file\": \"" << EscapeJsonString(config.antenna.pattern_file) << "\",\n";
    stream << "    \"polarization_file\": \"" << EscapeJsonString(config.antenna.polarization_file) << "\"\n";
    stream << "  },\n";

    stream << "  \"path_search\": {\n";
    stream << "    \"max_path_depth\": " << config.path_search.max_path_depth << ",\n";
    stream << "    \"max_reflection_count\": " << config.path_search.max_reflection_count << ",\n";
    stream << "    \"max_transmission_count\": " << config.path_search.max_transmission_count << ",\n";
    stream << "    \"max_diffraction_count\": " << config.path_search.max_diffraction_count << ",\n";
    stream << "    \"max_scattering_count\": " << config.path_search.max_scattering_count << ",\n";
    stream << "    \"max_candidate_face_hits\": " << config.path_search.max_candidate_face_hits << ",\n";
    stream << "    \"max_candidate_wedges\": " << config.path_search.max_candidate_wedges << ",\n";
    stream << "    \"enable_los\": " << (config.path_search.enable_los ? "true" : "false") << ",\n";
    stream << "    \"enable_reflection\": " << (config.path_search.enable_reflection ? "true" : "false") << ",\n";
    stream << "    \"enable_transmission\": " << (config.path_search.enable_transmission ? "true" : "false") << ",\n";
    stream << "    \"enable_diffraction\": " << (config.path_search.enable_diffraction ? "true" : "false") << ",\n";
    stream << "    \"enable_scattering\": " << (config.path_search.enable_scattering ? "true" : "false") << ",\n";
    stream << "    \"enable_mixed_path\": " << (config.path_search.enable_mixed_path ? "true" : "false") << ",\n";
    stream << "    \"keep_angle_metadata\": " << (config.path_search.keep_angle_metadata ? "true" : "false") << ",\n";
    stream << "    \"pruning_strategy\": \"" << EscapeJsonString(config.path_search.pruning_strategy) << "\",\n";
    stream << "    \"dedup_strategy\": \"" << EscapeJsonString(config.path_search.dedup_strategy) << "\"\n";
    stream << "  },\n";

    stream << "  \"em_solver\": {\n";
    stream << "    \"frequency_hz\": " << config.em_solver.frequency_hz << ",\n";
    stream << "    \"solver_mode\": \"" << EscapeJsonString(config.em_solver.solver_mode) << "\",\n";
    stream << "    \"enable_polarization\": " << (config.em_solver.enable_polarization ? "true" : "false") << "\n";
    stream << "  },\n";

    stream << "  \"output\": {\n";
    stream << "    \"output_directory\": \"" << EscapeJsonString(config.output.output_directory) << "\",\n";
    stream << "    \"export_paths\": " << (config.output.export_paths ? "true" : "false") << ",\n";
    stream << "    \"export_cir\": " << (config.output.export_cir ? "true" : "false") << ",\n";
    stream << "    \"export_pdp\": " << (config.output.export_pdp ? "true" : "false") << ",\n";
    stream << "    \"export_aps\": " << (config.output.export_aps ? "true" : "false") << ",\n";
    stream << "    \"export_debug_files\": " << (config.output.export_debug_files ? "true" : "false") << ",\n";
    stream << "    \"export_config_snapshot\": " << (config.output.export_config_snapshot ? "true" : "false") << "\n";
    stream << "  },\n";

    stream << "  \"validation\": {\n";
    stream << "    \"enable_basic_validation\": " << (config.validation.enable_basic_validation ? "true" : "false") << ",\n";
    stream << "    \"enable_reference_compare\": " << (config.validation.enable_reference_compare ? "true" : "false") << ",\n";
    stream << "    \"run_module1_self_check\": " << (config.validation.run_module1_self_check ? "true" : "false") << ",\n";
    stream << "    \"module1_invalid_transmission_case_file\": \"" << EscapeJsonString(config.validation.module1_invalid_transmission_case_file) << "\",\n";
    stream << "    \"module1_invalid_diffraction_case_file\": \"" << EscapeJsonString(config.validation.module1_invalid_diffraction_case_file) << "\",\n";
    stream << "    \"power_tolerance_db\": " << config.validation.power_tolerance_db << "\n";
    stream << "  },\n";

    stream << "  \"experiment\": {\n";
    stream << "    \"experiment_tag\": \"" << EscapeJsonString(config.experiment.experiment_tag) << "\",\n";
    stream << "    \"dataset_tag\": \"" << EscapeJsonString(config.experiment.dataset_tag) << "\"\n";
    stream << "  },\n";

    stream << "  \"numeric_tolerance\": {\n";
    stream << "    \"eps_length\": " << config.numeric_tolerance.eps_length << ",\n";
    stream << "    \"eps_angle\": " << config.numeric_tolerance.eps_angle << ",\n";
    stream << "    \"eps_intersection\": " << config.numeric_tolerance.eps_intersection << ",\n";
    stream << "    \"eps_normal\": " << config.numeric_tolerance.eps_normal << ",\n";
    stream << "    \"eps_deduplicate\": " << config.numeric_tolerance.eps_deduplicate << ",\n";
    stream << "    \"eps_power\": " << config.numeric_tolerance.eps_power << ",\n";
    stream << "    \"self_hit_ignore_distance\": " << config.numeric_tolerance.self_hit_ignore_distance << ",\n";
    stream << "    \"visibility_origin_offset\": " << config.numeric_tolerance.visibility_origin_offset << ",\n";
    stream << "    \"visibility_target_shrink\": " << config.numeric_tolerance.visibility_target_shrink << "\n";
    stream << "  }\n";
    stream << "}\n";

    return stream.str();
}

} // namespace rt
