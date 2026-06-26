// v9 step14: 替换手写JSON parser为nlohmann/json 3.5.0
// 改进: 标准JSON解析、合法JSON输出(无尾逗号)、字段round-trip、未知字段warning

#include "AppConfigJsonCodec.h"

#include "../../../tools/SDK/support/tinygltf/json.hpp"

#include <fstream>
#include <sstream>
#include <cstdio>

using nlohmann::json;

namespace rt {

// ── 辅助: 安全读取JSON字段, 缺失时保留默认值 ──

namespace {

template <typename T>
void ReadJsonField(const json& obj, const char* key, T& target) {
    if (obj.find(key) != obj.end()) {
        try { obj.at(key).get_to(target); } catch (...) {}
    }
}

void ReadJsonField(const json& obj, const char* key, bool& target) {
    if (obj.find(key) != obj.end()) {
        try { target = obj.at(key).get<bool>(); } catch (...) {}
    }
}

void ReadJsonField(const json& obj, const char* key, std::string& target) {
    if (obj.find(key) != obj.end()) {
        try { target = obj.at(key).get<std::string>(); } catch (...) {}
    }
}

// ── 读取AppConfig全字段 ──

AppConfigJsonDecodeResult PopulateFromJson(const json& root) {
    AppConfigJsonDecodeResult result;

    // app_runtime
    if (root.find("app_runtime") != root.end()) {
        auto& o = root["app_runtime"];
        ReadJsonField(o, "mode", result.config.app_runtime.mode);
        ReadJsonField(o, "log_level", result.config.app_runtime.log_level);
        ReadJsonField(o, "enable_console_logging", result.config.app_runtime.enable_console_logging);
        ReadJsonField(o, "enable_file_logging", result.config.app_runtime.enable_file_logging);
        ReadJsonField(o, "log_file_path", result.config.app_runtime.log_file_path);
        ReadJsonField(o, "config_snapshot_directory", result.config.app_runtime.config_snapshot_directory);
        ReadJsonField(o, "cache_directory", result.config.app_runtime.cache_directory);
        ReadJsonField(o, "run_id", result.config.app_runtime.run_id);
    }
    // scene_import
    if (root.find("scene_import") != root.end()) {
        auto& o = root["scene_import"];
        ReadJsonField(o, "source_file", result.config.scene_import.source_file);
        ReadJsonField(o, "source_format", result.config.scene_import.source_format);
        ReadJsonField(o, "coordinate_transform", result.config.scene_import.coordinate_transform);
        ReadJsonField(o, "scene_material_map_file", result.config.scene_import.scene_material_map_file);
        ReadJsonField(o, "normalize_object_names", result.config.scene_import.normalize_object_names);
    }
    // scene_preprocess
    if (root.find("scene_preprocess") != root.end()) {
        auto& o = root["scene_preprocess"];
        ReadJsonField(o, "rebuild_normals", result.config.scene_preprocess.rebuild_normals);
        ReadJsonField(o, "enable_wedge_build", result.config.scene_preprocess.enable_wedge_build);
        ReadJsonField(o, "enable_scene_cache", result.config.scene_preprocess.enable_scene_cache);
        ReadJsonField(o, "bvh_leaf_size", result.config.scene_preprocess.bvh_leaf_size);
    }
    // material
    if (root.find("material") != root.end()) {
        auto& o = root["material"];
        ReadJsonField(o, "material_database_file", result.config.material.material_database_file);
        ReadJsonField(o, "missing_material_policy", result.config.material.missing_material_policy);
    }
    // antenna (v10.2: 全局回退配置, 向后兼容)
    if (root.find("antenna") != root.end()) {
        auto& o = root["antenna"];
        ReadJsonField(o, "source_type", result.config.antenna.source_type);
        ReadJsonField(o, "pattern_file", result.config.antenna.pattern_file);
        ReadJsonField(o, "polarization_file", result.config.antenna.polarization_file);
        ReadJsonField(o, "forward_x", result.config.antenna.forward_x);
        ReadJsonField(o, "forward_y", result.config.antenna.forward_y);
        ReadJsonField(o, "forward_z", result.config.antenna.forward_z);
        ReadJsonField(o, "up_x", result.config.antenna.up_x);
        ReadJsonField(o, "up_y", result.config.antenna.up_y);
        ReadJsonField(o, "up_z", result.config.antenna.up_z);
    }
    // v10.2: tx_antenna 独立发射天线配置
    if (root.find("tx_antenna") != root.end()) {
        auto& o = root["tx_antenna"];
        ReadJsonField(o, "source_type", result.config.tx_antenna.source_type);
        ReadJsonField(o, "pattern_file", result.config.tx_antenna.pattern_file);
        ReadJsonField(o, "polarization_file", result.config.tx_antenna.polarization_file);
        ReadJsonField(o, "forward_x", result.config.tx_antenna.forward_x);
        ReadJsonField(o, "forward_y", result.config.tx_antenna.forward_y);
        ReadJsonField(o, "forward_z", result.config.tx_antenna.forward_z);
        ReadJsonField(o, "up_x", result.config.tx_antenna.up_x);
        ReadJsonField(o, "up_y", result.config.tx_antenna.up_y);
        ReadJsonField(o, "up_z", result.config.tx_antenna.up_z);
    }
    // v10.2: rx_antenna 独立接收天线配置
    if (root.find("rx_antenna") != root.end()) {
        auto& o = root["rx_antenna"];
        ReadJsonField(o, "source_type", result.config.rx_antenna.source_type);
        ReadJsonField(o, "pattern_file", result.config.rx_antenna.pattern_file);
        ReadJsonField(o, "polarization_file", result.config.rx_antenna.polarization_file);
        ReadJsonField(o, "forward_x", result.config.rx_antenna.forward_x);
        ReadJsonField(o, "forward_y", result.config.rx_antenna.forward_y);
        ReadJsonField(o, "forward_z", result.config.rx_antenna.forward_z);
        ReadJsonField(o, "up_x", result.config.rx_antenna.up_x);
        ReadJsonField(o, "up_y", result.config.rx_antenna.up_y);
        ReadJsonField(o, "up_z", result.config.rx_antenna.up_z);
    }
    // path_search
    if (root.find("path_search") != root.end()) {
        auto& o = root["path_search"];
        ReadJsonField(o, "max_path_depth", result.config.path_search.max_path_depth);
        ReadJsonField(o, "max_reflection_count", result.config.path_search.max_reflection_count);
        ReadJsonField(o, "max_transmission_count", result.config.path_search.max_transmission_count);
        ReadJsonField(o, "max_diffraction_count", result.config.path_search.max_diffraction_count);
        ReadJsonField(o, "max_scattering_count", result.config.path_search.max_scattering_count);
        ReadJsonField(o, "enable_los", result.config.path_search.enable_los);
        ReadJsonField(o, "max_consecutive_same_interaction", result.config.path_search.max_consecutive_same_interaction);
        // v9 step14: 新增剪枝/wedge配置字段
        ReadJsonField(o, "per_expander_keep_limit", result.config.path_search.per_expander_keep_limit);
        ReadJsonField(o, "per_state_keep_limit", result.config.path_search.per_state_keep_limit);
        ReadJsonField(o, "wedge_max_distance_m", result.config.path_search.wedge_max_distance_m);
        ReadJsonField(o, "wedge_max_candidates", result.config.path_search.wedge_max_candidates);
        ReadJsonField(o, "direction_closure_angle_tol_deg", result.config.path_search.direction_closure_angle_tol_deg);
        ReadJsonField(o, "snell_residual_tol", result.config.path_search.snell_residual_tol);
        ReadJsonField(o, "exhaustive_debug_mode", result.config.path_search.exhaustive_debug_mode);
        ReadJsonField(o, "allow_boundary_edge_diffraction", result.config.path_search.allow_boundary_edge_diffraction);
        ReadJsonField(o, "enable_lambertian_scattering", result.config.path_search.enable_lambertian_scattering);
        ReadJsonField(o, "scattering_coefficient", result.config.path_search.scattering_coefficient);
        ReadJsonField(o, "tx_x", result.config.path_search.tx_x);
        ReadJsonField(o, "tx_y", result.config.path_search.tx_y);
        ReadJsonField(o, "tx_z", result.config.path_search.tx_z);
        ReadJsonField(o, "rx_x", result.config.path_search.rx_x);
        ReadJsonField(o, "rx_y", result.config.path_search.rx_y);
        ReadJsonField(o, "rx_z", result.config.path_search.rx_z);
        // rx_list array (v10.2: 增加 per-Rx 天线覆盖字段)
        if (o.find("rx_list") != o.end() && o["rx_list"].is_array()) {
            for (auto& rxObj : o["rx_list"]) {
                RxTarget rx;
                ReadJsonField(rxObj, "id", rx.id);
                ReadJsonField(rxObj, "x", rx.x);
                ReadJsonField(rxObj, "y", rx.y);
                ReadJsonField(rxObj, "z", rx.z);
                // v10.2: per-Rx 天线覆盖 (可选)
                ReadJsonField(rxObj, "rx_source_type", rx.rx_source_type);
                ReadJsonField(rxObj, "rx_pattern_file", rx.rx_pattern_file);
                ReadJsonField(rxObj, "rx_polarization_file", rx.rx_polarization_file);
                ReadJsonField(rxObj, "rx_forward_x", rx.rx_forward_x);
                ReadJsonField(rxObj, "rx_forward_y", rx.rx_forward_y);
                ReadJsonField(rxObj, "rx_forward_z", rx.rx_forward_z);
                ReadJsonField(rxObj, "rx_up_x", rx.rx_up_x);
                ReadJsonField(rxObj, "rx_up_y", rx.rx_up_y);
                ReadJsonField(rxObj, "rx_up_z", rx.rx_up_z);
                if (!rx.id.empty()) result.config.path_search.rx_list.push_back(rx);
            }
        }
    }
    // sbr
    if (root.find("sbr") != root.end()) {
        auto& o = root["sbr"];
        ReadJsonField(o, "enabled", result.config.sbr.enabled);
        ReadJsonField(o, "trace_profile", result.config.sbr.trace_profile);
        ReadJsonField(o, "ray_count", result.config.sbr.ray_count);
        ReadJsonField(o, "max_ray_depth", result.config.sbr.max_ray_depth);
        ReadJsonField(o, "max_reflection_count", result.config.sbr.max_reflection_count);
        ReadJsonField(o, "max_transmission_count", result.config.sbr.max_transmission_count);
        ReadJsonField(o, "max_diffraction_count", result.config.sbr.max_diffraction_count);
        ReadJsonField(o, "ray_power_threshold_dB", result.config.sbr.ray_power_threshold_dB);
        ReadJsonField(o, "rx_sphere_radius_m", result.config.sbr.rx_sphere_radius_m);
        ReadJsonField(o, "auto_grid_bounds", result.config.sbr.auto_grid_bounds);
        ReadJsonField(o, "rx_grid_min_x", result.config.sbr.rx_grid_min_x);
        ReadJsonField(o, "rx_grid_max_x", result.config.sbr.rx_grid_max_x);
        ReadJsonField(o, "rx_grid_min_y", result.config.sbr.rx_grid_min_y);
        ReadJsonField(o, "rx_grid_max_y", result.config.sbr.rx_grid_max_y);
        ReadJsonField(o, "rx_grid_min_z", result.config.sbr.rx_grid_min_z);
        ReadJsonField(o, "rx_grid_max_z", result.config.sbr.rx_grid_max_z);
        ReadJsonField(o, "rx_grid_step_x", result.config.sbr.rx_grid_step_x);
        ReadJsonField(o, "rx_grid_step_y", result.config.sbr.rx_grid_step_y);
        ReadJsonField(o, "rx_grid_step_z", result.config.sbr.rx_grid_step_z);
        ReadJsonField(o, "tx_power_dBm", result.config.sbr.tx_power_dBm);
        ReadJsonField(o, "store_paths", result.config.sbr.store_paths);
        ReadJsonField(o, "wedge_max_distance_m", result.config.sbr.wedge_max_distance_m);
        ReadJsonField(o, "wedge_max_candidates", result.config.sbr.wedge_max_candidates);
        ReadJsonField(o, "deterministic_interaction_split", result.config.sbr.deterministic_interaction_split);
        ReadJsonField(o, "disable_no_new_hit_early_stop", result.config.sbr.disable_no_new_hit_early_stop);
        ReadJsonField(o, "max_paths_per_ray", result.config.sbr.max_paths_per_ray);
        ReadJsonField(o, "max_paths_per_rx", result.config.sbr.max_paths_per_rx);
        ReadJsonField(o, "enable_dynamic_rx_radius", result.config.sbr.enable_dynamic_rx_radius);
        ReadJsonField(o, "ray_tube_angle_rad", result.config.sbr.ray_tube_angle_rad);
        ReadJsonField(o, "ray_tube_radius_scale", result.config.sbr.ray_tube_radius_scale);
        ReadJsonField(o, "ray_tube_min_radius_m", result.config.sbr.ray_tube_min_radius_m);
        ReadJsonField(o, "ray_tube_max_radius_m", result.config.sbr.ray_tube_max_radius_m);
        ReadJsonField(o, "enable_wedge_tube_coupling", result.config.sbr.enable_wedge_tube_coupling);
        ReadJsonField(o, "wedge_tube_radius_scale", result.config.sbr.wedge_tube_radius_scale);
        ReadJsonField(o, "diffraction_rays_per_event", result.config.sbr.diffraction_rays_per_event);
        ReadJsonField(o, "enable_path_dedup", result.config.sbr.enable_path_dedup);
        ReadJsonField(o, "enable_path_similarity_pruning", result.config.sbr.enable_path_similarity_pruning);
        ReadJsonField(o, "path_similarity_length_tol_m", result.config.sbr.path_similarity_length_tol_m);
        ReadJsonField(o, "path_top_n_per_rx", result.config.sbr.path_top_n_per_rx);
        ReadJsonField(o, "enable_path_residual_filter", result.config.sbr.enable_path_residual_filter);
        ReadJsonField(o, "path_geometry_residual_tol", result.config.sbr.path_geometry_residual_tol);
        ReadJsonField(o, "reflection_residual_tol_m", result.config.sbr.reflection_residual_tol_m);
        ReadJsonField(o, "snell_residual_tol", result.config.sbr.snell_residual_tol);
        ReadJsonField(o, "keller_residual_tol", result.config.sbr.keller_residual_tol);
    }
    // em_solver (v10.2: 增加 APS 网格和 MEG 配置)
    if (root.find("em_solver") != root.end()) {
        auto& o = root["em_solver"];
        ReadJsonField(o, "frequency_hz", result.config.em_solver.frequency_hz);
        ReadJsonField(o, "solver_mode", result.config.em_solver.solver_mode);
        ReadJsonField(o, "aps_theta_bins", result.config.em_solver.aps_theta_bins);
        ReadJsonField(o, "aps_phi_bins", result.config.em_solver.aps_phi_bins);
        ReadJsonField(o, "aps_export_2d_grid", result.config.em_solver.aps_export_2d_grid);
        ReadJsonField(o, "compute_meg", result.config.em_solver.compute_meg);
    }
    // output
    if (root.find("output") != root.end()) {
        auto& o = root["output"];
        ReadJsonField(o, "export_paths", result.config.output.export_paths);
        ReadJsonField(o, "export_cir", result.config.output.export_cir);
        ReadJsonField(o, "export_pdp", result.config.output.export_pdp);
        ReadJsonField(o, "export_aps", result.config.output.export_aps);
        ReadJsonField(o, "export_config_snapshot", result.config.output.export_config_snapshot);
    }
    // validation
    if (root.find("validation") != root.end()) {
        auto& o = root["validation"];
        ReadJsonField(o, "enable_basic_validation", result.config.validation.enable_basic_validation);
        ReadJsonField(o, "enable_reference_compare", result.config.validation.enable_reference_compare);
        ReadJsonField(o, "run_module1_self_check", result.config.validation.run_module1_self_check);
        ReadJsonField(o, "power_tolerance_db", result.config.validation.power_tolerance_db);
    }
    // experiment
    if (root.find("experiment") != root.end()) {
        auto& o = root["experiment"];
        ReadJsonField(o, "experiment_tag", result.config.experiment.experiment_tag);
        ReadJsonField(o, "dataset_tag", result.config.experiment.dataset_tag);
    }
    // pipeline (v8)
    if (root.find("pipeline") != root.end()) {
        auto& o = root["pipeline"];
        ReadJsonField(o, "enable_stage0_precompute", result.config.pipeline.enable_stage0_precompute);
        ReadJsonField(o, "enable_pvs", result.config.pipeline.enable_pvs);
        ReadJsonField(o, "enable_edge_adjacency", result.config.pipeline.enable_edge_adjacency);
        ReadJsonField(o, "enable_angular_grid", result.config.pipeline.enable_angular_grid);
        ReadJsonField(o, "enable_stage4_precise_em", result.config.pipeline.enable_stage4_precise_em);
        ReadJsonField(o, "max_paths_per_rx", result.config.pipeline.max_paths_per_rx);
    }
    // v9 C: frequency_sweep
    if (root.find("frequency_sweep") != root.end()) {
        auto& o = root["frequency_sweep"];
        ReadJsonField(o, "enabled", result.config.frequency_sweep.enabled);
        ReadJsonField(o, "center_hz", result.config.frequency_sweep.center_hz);
        ReadJsonField(o, "bandwidth_hz", result.config.frequency_sweep.bandwidth_hz);
        ReadJsonField(o, "point_count", result.config.frequency_sweep.point_count);
        ReadJsonField(o, "spacing", result.config.frequency_sweep.spacing);
        ReadJsonField(o, "retrace_per_frequency", result.config.frequency_sweep.retrace_per_frequency);
        ReadJsonField(o, "mode", result.config.frequency_sweep.mode);
    }
    // v9 C: channel_observation
    if (root.find("channel_observation") != root.end()) {
        auto& o = root["channel_observation"];
        ReadJsonField(o, "export_ideal_delta_cir", result.config.channel_observation.export_ideal_delta_cir);
        ReadJsonField(o, "export_sampled_cfr", result.config.channel_observation.export_sampled_cfr);
        ReadJsonField(o, "export_observed_cir_ifft", result.config.channel_observation.export_observed_cir_ifft);
        ReadJsonField(o, "delay_bin_s", result.config.channel_observation.delay_bin_s);
        ReadJsonField(o, "window_type", result.config.channel_observation.window_type);
        ReadJsonField(o, "ifft_convention", result.config.channel_observation.ifft_convention);
    }
    // numeric_tolerance
    if (root.find("numeric_tolerance") != root.end()) {
        auto& o = root["numeric_tolerance"];
        ReadJsonField(o, "eps_length", result.config.numeric_tolerance.eps_length);
        ReadJsonField(o, "eps_angle", result.config.numeric_tolerance.eps_angle);
        ReadJsonField(o, "eps_intersection", result.config.numeric_tolerance.eps_intersection);
        ReadJsonField(o, "eps_normal", result.config.numeric_tolerance.eps_normal);
        ReadJsonField(o, "eps_deduplicate", result.config.numeric_tolerance.eps_deduplicate);
        ReadJsonField(o, "eps_power", result.config.numeric_tolerance.eps_power);
        ReadJsonField(o, "self_hit_ignore_distance", result.config.numeric_tolerance.self_hit_ignore_distance);
        ReadJsonField(o, "visibility_origin_offset", result.config.numeric_tolerance.visibility_origin_offset);
        ReadJsonField(o, "visibility_target_shrink", result.config.numeric_tolerance.visibility_target_shrink);
    }

    result.succeeded = true;
    return result;
}

} // namespace

// ── 公共接口 ──

AppConfigJsonDecodeResult DecodeAppConfigFromJsonFile(const std::string& filePath) {
    AppConfigJsonDecodeResult result;

    try {
        std::ifstream f(filePath);
        if (!f.is_open()) {
            result.error_message = "Cannot open config file: " + filePath;
            return result;
        }
        json root = json::parse(f);
        result = PopulateFromJson(root);
    } catch (const json::parse_error& e) {
        result.error_message = std::string("JSON parse error: ") + e.what();
    } catch (const std::exception& e) {
        result.error_message = std::string("Config read error: ") + e.what();
    }

    return result;
}

std::string EncodeAppConfigToJsonString(const AppConfig& config) {
    json root;

    // app_runtime
    root["app_runtime"] = {
        {"mode", config.app_runtime.mode},
        {"log_level", config.app_runtime.log_level},
        {"enable_console_logging", config.app_runtime.enable_console_logging},
        {"enable_file_logging", config.app_runtime.enable_file_logging},
        {"log_file_path", config.app_runtime.log_file_path},
        {"config_snapshot_directory", config.app_runtime.config_snapshot_directory},
        {"cache_directory", config.app_runtime.cache_directory},
        {"run_id", config.app_runtime.run_id}
    };

    // scene_import
    root["scene_import"] = {
        {"source_file", config.scene_import.source_file},
        {"source_format", config.scene_import.source_format},
        {"coordinate_transform", config.scene_import.coordinate_transform},
        {"scene_material_map_file", config.scene_import.scene_material_map_file},
        {"normalize_object_names", config.scene_import.normalize_object_names}
    };

    // scene_preprocess
    root["scene_preprocess"] = {
        {"rebuild_normals", config.scene_preprocess.rebuild_normals},
        {"enable_wedge_build", config.scene_preprocess.enable_wedge_build},
        {"enable_scene_cache", config.scene_preprocess.enable_scene_cache},
        {"bvh_leaf_size", config.scene_preprocess.bvh_leaf_size}
    };

    // material
    root["material"] = {
        {"material_database_file", config.material.material_database_file},
        {"missing_material_policy", config.material.missing_material_policy}
    };

    // antenna
    root["antenna"] = {
        {"source_type", config.antenna.source_type},
        {"pattern_file", config.antenna.pattern_file},
        {"polarization_file", config.antenna.polarization_file},
        {"forward_x", config.antenna.forward_x},
        {"forward_y", config.antenna.forward_y},
        {"forward_z", config.antenna.forward_z},
        {"up_x", config.antenna.up_x},
        {"up_y", config.antenna.up_y},
        {"up_z", config.antenna.up_z}
    };

    // path_search (v9: includes new fields)
    root["path_search"] = {
        {"max_path_depth", config.path_search.max_path_depth},
        {"max_reflection_count", config.path_search.max_reflection_count},
        {"max_transmission_count", config.path_search.max_transmission_count},
        {"max_diffraction_count", config.path_search.max_diffraction_count},
        {"max_scattering_count", config.path_search.max_scattering_count},
        {"enable_los", config.path_search.enable_los},
        {"max_consecutive_same_interaction", config.path_search.max_consecutive_same_interaction},
        {"per_expander_keep_limit", config.path_search.per_expander_keep_limit},
        {"per_state_keep_limit", config.path_search.per_state_keep_limit},
        {"wedge_max_distance_m", config.path_search.wedge_max_distance_m},
        {"wedge_max_candidates", config.path_search.wedge_max_candidates},
        {"direction_closure_angle_tol_deg", config.path_search.direction_closure_angle_tol_deg},
        {"snell_residual_tol", config.path_search.snell_residual_tol},
        {"exhaustive_debug_mode", config.path_search.exhaustive_debug_mode},
        {"allow_boundary_edge_diffraction", config.path_search.allow_boundary_edge_diffraction},
        {"enable_lambertian_scattering", config.path_search.enable_lambertian_scattering},
        {"scattering_coefficient", config.path_search.scattering_coefficient},
        {"tx_x", config.path_search.tx_x},
        {"tx_y", config.path_search.tx_y},
        {"tx_z", config.path_search.tx_z},
        {"rx_x", config.path_search.rx_x},
        {"rx_y", config.path_search.rx_y},
        {"rx_z", config.path_search.rx_z}
    };

    // rx_list array
    if (!config.path_search.rx_list.empty()) {
        json rxArr = json::array();
        for (auto& rx : config.path_search.rx_list) {
            rxArr.push_back({
                {"id", rx.id},
                {"x", rx.x},
                {"y", rx.y},
                {"z", rx.z}
            });
        }
        root["path_search"]["rx_list"] = rxArr;
    }

    // sbr
    root["sbr"] = {
        {"enabled", config.sbr.enabled},
        {"trace_profile", config.sbr.trace_profile},
        {"ray_count", config.sbr.ray_count},
        {"max_ray_depth", config.sbr.max_ray_depth},
        {"max_reflection_count", config.sbr.max_reflection_count},
        {"max_transmission_count", config.sbr.max_transmission_count},
        {"max_diffraction_count", config.sbr.max_diffraction_count},
        {"ray_power_threshold_dB", config.sbr.ray_power_threshold_dB},
        {"rx_sphere_radius_m", config.sbr.rx_sphere_radius_m},
        {"auto_grid_bounds", config.sbr.auto_grid_bounds},
        {"rx_grid_min_x", config.sbr.rx_grid_min_x},
        {"rx_grid_max_x", config.sbr.rx_grid_max_x},
        {"rx_grid_min_y", config.sbr.rx_grid_min_y},
        {"rx_grid_max_y", config.sbr.rx_grid_max_y},
        {"rx_grid_min_z", config.sbr.rx_grid_min_z},
        {"rx_grid_max_z", config.sbr.rx_grid_max_z},
        {"rx_grid_step_x", config.sbr.rx_grid_step_x},
        {"rx_grid_step_y", config.sbr.rx_grid_step_y},
        {"rx_grid_step_z", config.sbr.rx_grid_step_z},
        {"tx_power_dBm", config.sbr.tx_power_dBm},
        {"store_paths", config.sbr.store_paths},
        {"wedge_max_distance_m", config.sbr.wedge_max_distance_m},
        {"wedge_max_candidates", config.sbr.wedge_max_candidates},
        {"deterministic_interaction_split", config.sbr.deterministic_interaction_split},
        {"disable_no_new_hit_early_stop", config.sbr.disable_no_new_hit_early_stop},
        {"max_paths_per_ray", config.sbr.max_paths_per_ray},
        {"max_paths_per_rx", config.sbr.max_paths_per_rx},
        {"enable_dynamic_rx_radius", config.sbr.enable_dynamic_rx_radius},
        {"ray_tube_angle_rad", config.sbr.ray_tube_angle_rad},
        {"ray_tube_radius_scale", config.sbr.ray_tube_radius_scale},
        {"ray_tube_min_radius_m", config.sbr.ray_tube_min_radius_m},
        {"ray_tube_max_radius_m", config.sbr.ray_tube_max_radius_m},
        {"enable_wedge_tube_coupling", config.sbr.enable_wedge_tube_coupling},
        {"wedge_tube_radius_scale", config.sbr.wedge_tube_radius_scale},
        {"diffraction_rays_per_event", config.sbr.diffraction_rays_per_event},
        {"enable_path_dedup", config.sbr.enable_path_dedup},
        {"enable_path_similarity_pruning", config.sbr.enable_path_similarity_pruning},
        {"path_similarity_length_tol_m", config.sbr.path_similarity_length_tol_m},
        {"path_top_n_per_rx", config.sbr.path_top_n_per_rx},
        {"enable_path_residual_filter", config.sbr.enable_path_residual_filter},
        {"path_geometry_residual_tol", config.sbr.path_geometry_residual_tol},
        {"reflection_residual_tol_m", config.sbr.reflection_residual_tol_m},
        {"snell_residual_tol", config.sbr.snell_residual_tol},
        {"keller_residual_tol", config.sbr.keller_residual_tol}
    };

    // em_solver (v10.2: APS and MEG fields)
    root["em_solver"] = {
        {"frequency_hz", config.em_solver.frequency_hz},
        {"solver_mode", config.em_solver.solver_mode},
        {"aps_theta_bins", config.em_solver.aps_theta_bins},
        {"aps_phi_bins", config.em_solver.aps_phi_bins},
        {"aps_export_2d_grid", config.em_solver.aps_export_2d_grid},
        {"compute_meg", config.em_solver.compute_meg}
    };

    // output
    root["output"] = {
        {"export_paths", config.output.export_paths},
        {"export_cir", config.output.export_cir},
        {"export_pdp", config.output.export_pdp},
        {"export_aps", config.output.export_aps},
        {"export_config_snapshot", config.output.export_config_snapshot}
    };

    // validation
    root["validation"] = {
        {"enable_basic_validation", config.validation.enable_basic_validation},
        {"enable_reference_compare", config.validation.enable_reference_compare},
        {"run_module1_self_check", config.validation.run_module1_self_check},
        {"power_tolerance_db", config.validation.power_tolerance_db}
    };

    // experiment
    root["experiment"] = {
        {"experiment_tag", config.experiment.experiment_tag},
        {"dataset_tag", config.experiment.dataset_tag}
    };

    // pipeline (v8)
    root["pipeline"] = {
        {"enable_stage0_precompute", config.pipeline.enable_stage0_precompute},
        {"enable_pvs", config.pipeline.enable_pvs},
        {"enable_edge_adjacency", config.pipeline.enable_edge_adjacency},
        {"enable_angular_grid", config.pipeline.enable_angular_grid},
        {"enable_stage4_precise_em", config.pipeline.enable_stage4_precise_em},
        {"max_paths_per_rx", config.pipeline.max_paths_per_rx}
    };

    // v9 C: frequency_sweep
    root["frequency_sweep"] = {
        {"enabled", config.frequency_sweep.enabled},
        {"center_hz", config.frequency_sweep.center_hz},
        {"bandwidth_hz", config.frequency_sweep.bandwidth_hz},
        {"point_count", config.frequency_sweep.point_count},
        {"spacing", config.frequency_sweep.spacing},
        {"retrace_per_frequency", config.frequency_sweep.retrace_per_frequency},
        {"mode", config.frequency_sweep.mode}
    };
    // v9 C: channel_observation
    root["channel_observation"] = {
        {"export_ideal_delta_cir", config.channel_observation.export_ideal_delta_cir},
        {"export_sampled_cfr", config.channel_observation.export_sampled_cfr},
        {"export_observed_cir_ifft", config.channel_observation.export_observed_cir_ifft},
        {"delay_bin_s", config.channel_observation.delay_bin_s},
        {"window_type", config.channel_observation.window_type},
        {"ifft_convention", config.channel_observation.ifft_convention}
    };

    // numeric_tolerance
    root["numeric_tolerance"] = {
        {"eps_length", config.numeric_tolerance.eps_length},
        {"eps_angle", config.numeric_tolerance.eps_angle},
        {"eps_intersection", config.numeric_tolerance.eps_intersection},
        {"eps_normal", config.numeric_tolerance.eps_normal},
        {"eps_deduplicate", config.numeric_tolerance.eps_deduplicate},
        {"eps_power", config.numeric_tolerance.eps_power},
        {"self_hit_ignore_distance", config.numeric_tolerance.self_hit_ignore_distance},
        {"visibility_origin_offset", config.numeric_tolerance.visibility_origin_offset},
        {"visibility_target_shrink", config.numeric_tolerance.visibility_target_shrink}
    };

    return root.dump(2); // 2-space indent, valid JSON
}

} // namespace rt
