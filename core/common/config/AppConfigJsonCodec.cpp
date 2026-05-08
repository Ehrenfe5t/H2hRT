// 文件目标：
// - 实现模块1统一 JSON 编解码层。
//
// 主要功能：
// - 把配置读取逻辑集中在 codec 层；
// - 把配置快照写出逻辑与读取逻辑绑定到同一份 schema；
// - 为后续替换更强 JSON 实现预留稳定接口。

#include "AppConfigJsonCodec.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace rt {

namespace {

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

std::size_t SkipWhitespace(const std::string& text, std::size_t index)
{
    while (index < text.size() && std::isspace(static_cast<unsigned char>(text[index])) != 0)
    {
        ++index;
    }
    return index;
}

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

void ReadOptionalString(const std::string& objectBody, const std::string& key, std::string& target)
{
    std::string value;
    if (ExtractRawValue(objectBody, key, value))
    {
        target = value;
    }
}

void ReadOptionalBool(const std::string& objectBody, const std::string& key, bool& target)
{
    std::string value;
    if (ExtractRawValue(objectBody, key, value))
    {
        target = (value == "true" || value == "TRUE" || value == "True");
    }
}

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
        ReadOptionalString(body, "scene_material_map_file", config.scene_import.scene_material_map_file);
        ReadOptionalBool(body, "normalize_object_names", config.scene_import.normalize_object_names);
        ReadOptionalBool(body, "allow_name_auto_cleanup", config.scene_import.allow_name_auto_cleanup);
    }

    if (ExtractObjectBody(text, "scene_preprocess", body))
    {
        ReadOptionalBool(body, "rebuild_normals", config.scene_preprocess.rebuild_normals);
        ReadOptionalBool(body, "enable_wedge_build", config.scene_preprocess.enable_wedge_build);
        ReadOptionalBool(body, "enable_scene_cache", config.scene_preprocess.enable_scene_cache);
        ReadOptionalBool(body, "enable_bvh_bruteforce_validation", config.scene_preprocess.enable_bvh_bruteforce_validation);
        ReadOptionalBool(body, "filter_non_manifold_wedge_sources", config.scene_preprocess.filter_non_manifold_wedge_sources);
        ReadOptionalBool(body, "skip_coplanar_edges_for_wedge", config.scene_preprocess.skip_coplanar_edges_for_wedge);
        ReadOptionalString(body, "preprocess_mode", config.scene_preprocess.preprocess_mode);
        ReadOptionalString(body, "scene_cache_format_version", config.scene_preprocess.scene_cache_format_version);
        ReadOptionalString(body, "scene_preprocess_algorithm_version", config.scene_preprocess.scene_preprocess_algorithm_version);
        ReadOptionalNumber(body, "wedge_min_angle_deg", config.scene_preprocess.wedge_min_angle_deg);
        ReadOptionalNumber(body, "wedge_max_angle_deg", config.scene_preprocess.wedge_max_angle_deg);
        ReadOptionalNumber(body, "bvh_leaf_size", config.scene_preprocess.bvh_leaf_size);
        ReadOptionalNumber(body, "bvh_bruteforce_sample_count", config.scene_preprocess.bvh_bruteforce_sample_count);
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
        ReadOptionalNumber(body, "max_consecutive_same_interaction", config.path_search.max_consecutive_same_interaction);
        ReadOptionalString(body, "pruning_strategy", config.path_search.pruning_strategy);
        ReadOptionalString(body, "dedup_strategy", config.path_search.dedup_strategy);
        ReadOptionalNumber(body, "debug_tx_x", config.path_search.debug_tx_x);
        ReadOptionalNumber(body, "debug_tx_y", config.path_search.debug_tx_y);
        ReadOptionalNumber(body, "debug_tx_z", config.path_search.debug_tx_z);
        ReadOptionalNumber(body, "debug_rx_x", config.path_search.debug_rx_x);
        ReadOptionalNumber(body, "debug_rx_y", config.path_search.debug_rx_y);
        ReadOptionalNumber(body, "debug_rx_z", config.path_search.debug_rx_z);
    }

    if (ExtractObjectBody(text, "sbr", body))
    {
        ReadOptionalBool(body, "enabled", config.sbr.enabled);
        ReadOptionalNumber(body, "ray_count", config.sbr.ray_count);
        ReadOptionalNumber(body, "max_ray_depth", config.sbr.max_ray_depth);
        ReadOptionalNumber(body, "max_reflection_count", config.sbr.max_reflection_count);
        ReadOptionalNumber(body, "max_transmission_count", config.sbr.max_transmission_count);
        ReadOptionalNumber(body, "max_diffraction_count", config.sbr.max_diffraction_count);
        ReadOptionalBool(body, "enable_transmission", config.sbr.enable_transmission);
        ReadOptionalBool(body, "enable_diffraction", config.sbr.enable_diffraction);
        ReadOptionalNumber(body, "ray_power_threshold_linear", config.sbr.ray_power_threshold_linear);
        ReadOptionalNumber(body, "rx_sphere_radius_m", config.sbr.rx_sphere_radius_m);
        ReadOptionalBool(body, "auto_grid_bounds", config.sbr.auto_grid_bounds);
        ReadOptionalNumber(body, "grid_margin_m", config.sbr.grid_margin_m);
        ReadOptionalNumber(body, "rx_grid_min_x", config.sbr.rx_grid_min_x);
        ReadOptionalNumber(body, "rx_grid_max_x", config.sbr.rx_grid_max_x);
        ReadOptionalNumber(body, "rx_grid_min_y", config.sbr.rx_grid_min_y);
        ReadOptionalNumber(body, "rx_grid_max_y", config.sbr.rx_grid_max_y);
        ReadOptionalNumber(body, "rx_grid_min_z", config.sbr.rx_grid_min_z);
        ReadOptionalNumber(body, "rx_grid_max_z", config.sbr.rx_grid_max_z);
        ReadOptionalNumber(body, "rx_grid_step_x", config.sbr.rx_grid_step_x);
        ReadOptionalNumber(body, "rx_grid_step_y", config.sbr.rx_grid_step_y);
        ReadOptionalNumber(body, "rx_grid_step_z", config.sbr.rx_grid_step_z);
        ReadOptionalNumber(body, "tx_power_w", config.sbr.tx_power_w);
        ReadOptionalBool(body, "store_paths", config.sbr.store_paths);
        ReadOptionalNumber(body, "wedge_max_distance_m", config.sbr.wedge_max_distance_m);
        ReadOptionalNumber(body, "wedge_sample_count", config.sbr.wedge_sample_count);
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

AppConfigJsonDecodeResult DecodeAppConfigFromJsonFile(const std::string& filePath)
{
    AppConfigJsonDecodeResult result;

    std::string content;
    if (!ReadWholeFile(filePath, content))
    {
        result.error_message = "Unable to read JSON config file content.";
        return result;
    }

    if (content.find('{') == std::string::npos)
    {
        result.error_message = "JSON object root not found in config file.";
        return result;
    }

    if (!PopulateAppConfigFromJsonText(content, result.config))
    {
        result.error_message = "Failed while mapping JSON text into AppConfig.";
        return result;
    }

    result.succeeded = true;
    return result;
}

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
    stream << "    \"scene_material_map_file\": \"" << EscapeJsonString(config.scene_import.scene_material_map_file) << "\",\n";
    stream << "    \"normalize_object_names\": " << (config.scene_import.normalize_object_names ? "true" : "false") << ",\n";
    stream << "    \"allow_name_auto_cleanup\": " << (config.scene_import.allow_name_auto_cleanup ? "true" : "false") << "\n";
    stream << "  },\n";

    stream << "  \"scene_preprocess\": {\n";
    stream << "    \"rebuild_normals\": " << (config.scene_preprocess.rebuild_normals ? "true" : "false") << ",\n";
    stream << "    \"enable_wedge_build\": " << (config.scene_preprocess.enable_wedge_build ? "true" : "false") << ",\n";
    stream << "    \"enable_scene_cache\": " << (config.scene_preprocess.enable_scene_cache ? "true" : "false") << ",\n";
    stream << "    \"enable_bvh_bruteforce_validation\": " << (config.scene_preprocess.enable_bvh_bruteforce_validation ? "true" : "false") << ",\n";
    stream << "    \"filter_non_manifold_wedge_sources\": " << (config.scene_preprocess.filter_non_manifold_wedge_sources ? "true" : "false") << ",\n";
    stream << "    \"skip_coplanar_edges_for_wedge\": " << (config.scene_preprocess.skip_coplanar_edges_for_wedge ? "true" : "false") << ",\n";
    stream << "    \"preprocess_mode\": \"" << EscapeJsonString(config.scene_preprocess.preprocess_mode) << "\",\n";
    stream << "    \"scene_cache_format_version\": \"" << EscapeJsonString(config.scene_preprocess.scene_cache_format_version) << "\",\n";
    stream << "    \"scene_preprocess_algorithm_version\": \"" << EscapeJsonString(config.scene_preprocess.scene_preprocess_algorithm_version) << "\",\n";
    stream << "    \"wedge_min_angle_deg\": " << config.scene_preprocess.wedge_min_angle_deg << ",\n";
    stream << "    \"wedge_max_angle_deg\": " << config.scene_preprocess.wedge_max_angle_deg << ",\n";
    stream << "    \"bvh_leaf_size\": " << config.scene_preprocess.bvh_leaf_size << ",\n";
    stream << "    \"bvh_bruteforce_sample_count\": " << config.scene_preprocess.bvh_bruteforce_sample_count << "\n";
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
    stream << "    \"max_consecutive_same_interaction\": " << config.path_search.max_consecutive_same_interaction << ",\n";
    stream << "    \"pruning_strategy\": \"" << EscapeJsonString(config.path_search.pruning_strategy) << "\",\n";
    stream << "    \"dedup_strategy\": \"" << EscapeJsonString(config.path_search.dedup_strategy) << "\",\n";
    stream << "    \"debug_tx_x\": " << config.path_search.debug_tx_x << ",\n";
    stream << "    \"debug_tx_y\": " << config.path_search.debug_tx_y << ",\n";
    stream << "    \"debug_tx_z\": " << config.path_search.debug_tx_z << ",\n";
    stream << "    \"debug_rx_x\": " << config.path_search.debug_rx_x << ",\n";
    stream << "    \"debug_rx_y\": " << config.path_search.debug_rx_y << ",\n";
    stream << "    \"debug_rx_z\": " << config.path_search.debug_rx_z << "\n";
    stream << "  },\n";

    stream << "  \"sbr\": {\n";
    stream << "    \"enabled\": " << (config.sbr.enabled ? "true" : "false") << ",\n";
    stream << "    \"ray_count\": " << config.sbr.ray_count << ",\n";
    stream << "    \"max_ray_depth\": " << config.sbr.max_ray_depth << ",\n";
    stream << "    \"max_reflection_count\": " << config.sbr.max_reflection_count << ",\n";
    stream << "    \"max_transmission_count\": " << config.sbr.max_transmission_count << ",\n";
    stream << "    \"max_diffraction_count\": " << config.sbr.max_diffraction_count << ",\n";
    stream << "    \"enable_transmission\": " << (config.sbr.enable_transmission ? "true" : "false") << ",\n";
    stream << "    \"enable_diffraction\": " << (config.sbr.enable_diffraction ? "true" : "false") << ",\n";
    stream << "    \"ray_power_threshold_linear\": " << config.sbr.ray_power_threshold_linear << ",\n";
    stream << "    \"rx_sphere_radius_m\": " << config.sbr.rx_sphere_radius_m << ",\n";
    stream << "    \"auto_grid_bounds\": " << (config.sbr.auto_grid_bounds ? "true" : "false") << ",\n";
    stream << "    \"grid_margin_m\": " << config.sbr.grid_margin_m << ",\n";
    stream << "    \"rx_grid_min_x\": " << config.sbr.rx_grid_min_x << ",\n";
    stream << "    \"rx_grid_max_x\": " << config.sbr.rx_grid_max_x << ",\n";
    stream << "    \"rx_grid_min_y\": " << config.sbr.rx_grid_min_y << ",\n";
    stream << "    \"rx_grid_max_y\": " << config.sbr.rx_grid_max_y << ",\n";
    stream << "    \"rx_grid_min_z\": " << config.sbr.rx_grid_min_z << ",\n";
    stream << "    \"rx_grid_max_z\": " << config.sbr.rx_grid_max_z << ",\n";
    stream << "    \"rx_grid_step_x\": " << config.sbr.rx_grid_step_x << ",\n";
    stream << "    \"rx_grid_step_y\": " << config.sbr.rx_grid_step_y << ",\n";
    stream << "    \"rx_grid_step_z\": " << config.sbr.rx_grid_step_z << ",\n";
    stream << "    \"tx_power_w\": " << config.sbr.tx_power_w << ",\n";
    stream << "    \"store_paths\": " << (config.sbr.store_paths ? "true" : "false") << ",\n";
    stream << "    \"wedge_max_distance_m\": " << config.sbr.wedge_max_distance_m << ",\n";
    stream << "    \"wedge_sample_count\": " << config.sbr.wedge_sample_count << "\n";
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
