// 文件目标：
// - 定义模块1统一配置对象及其直接相关的校验结果结构。
//
// 主要功能：
// - 提供 AppConfig 作为系统唯一高层配置真源；
// - 汇总当前批次已冻结的基础配置块；
// - 为配置加载器、配置校验器以及后续模块提供稳定接口。

#pragma once

#include "../numeric/NumericToleranceConfig.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 应用运行时配置。
/// </summary>
struct AppRuntimeConfig {
    std::string mode = "debug";
    std::string log_level = "INFO";
    bool enable_console_logging = true;
    bool enable_file_logging = true;
    std::string log_file_path = "output/logs/rt.log";
    std::string config_snapshot_directory = "output/config_snapshots";
    std::string cache_directory = "output/cache";
    std::string run_id = "local-run";
    int worker_threads = 1;
};

/// <summary>
/// 场景导入配置。
/// </summary>
struct SceneImportConfig {
    std::string source_file;
    std::string source_format = "obj";
    std::string scene_material_map_file = "configs/scenes/scene_material_map.json";
    bool normalize_object_names = true;
    bool allow_name_auto_cleanup = true;
};

/// <summary>
/// 场景预处理配置。
/// </summary>
struct ScenePreprocessConfig {
    bool rebuild_normals = false;
    bool enable_wedge_build = true;
    bool enable_scene_cache = false;
    bool enable_bvh_bruteforce_validation = true;
    bool filter_non_manifold_wedge_sources = true;
    bool skip_coplanar_edges_for_wedge = true;
    std::string preprocess_mode = "debug";
    std::string scene_cache_format_version = "1.0.0";
    std::string scene_preprocess_algorithm_version = "batch4-v1";
    double wedge_min_angle_deg = 1.0;
    double wedge_max_angle_deg = 179.0;
    int bvh_leaf_size = 8;
    int bvh_bruteforce_sample_count = 16;
};

/// <summary>
/// 材料系统配置。
/// </summary>
struct MaterialConfig {
    std::string material_database_file;
    std::string material_mapping_file;
    std::string frequency_query_mode = "exact";
    bool allow_material_fallback = false;
    std::string default_background_medium = "air";
};

/// <summary>
/// 天线输入配置。
/// </summary>
struct AntennaConfig {
    std::string source_type = "Ideal";
    std::string pattern_file;
    std::string polarization_file;
};

/// <summary>
/// 几何寻径配置。
/// </summary>
struct PathSearchConfig {
    int max_path_depth = 2;
    int max_reflection_count = 2;
    int max_transmission_count = 0;
    int max_diffraction_count = 0;
    int max_scattering_count = 0;
    int max_candidate_face_hits = 64;
    int max_candidate_wedges = 64;
    bool enable_los = true;
    bool enable_reflection = true;
    bool enable_transmission = false;
    bool enable_diffraction = false;
    bool enable_scattering = false;
    bool enable_mixed_path = true;
    bool keep_angle_metadata = true;
    std::string pruning_strategy = "basic";
    std::string dedup_strategy = "signature";
};

/// <summary>
/// 电磁求解配置。
/// </summary>
struct EMSolverConfig {
    double frequency_hz = 2.4e9;
    std::string solver_mode = "Precise";
    bool enable_polarization = true;
};

/// <summary>
/// 输出控制配置。
/// </summary>
struct OutputConfig {
    std::string output_directory = "output";
    bool export_paths = true;
    bool export_cir = false;
    bool export_pdp = false;
    bool export_aps = false;
    bool export_debug_files = false;
    bool export_config_snapshot = true;
};

/// <summary>
/// 验证与回归控制配置。
/// </summary>
struct ValidationConfig {
    bool enable_basic_validation = true;
    bool enable_reference_compare = false;
    bool run_module1_self_check = true;
    std::string module1_invalid_transmission_case_file = "configs/app/invalid_transmission_missing_mapping.json";
    std::string module1_invalid_diffraction_case_file = "configs/app/invalid_diffraction_without_wedge.json";
    double power_tolerance_db = 0.1;
};

/// <summary>
/// 实验标识与批处理控制配置。
/// </summary>
struct ExperimentConfig {
    std::string experiment_tag = "bootstrap";
    std::string dataset_tag = "demo";
};

/// <summary>
/// RT 系统统一顶层配置对象。
/// </summary>
/// <remarks>
/// 该结构是当前系统唯一高层配置真源。
/// 后续新增功能应优先扩展其子配置块，而不是在模块内部另造私有配置入口。
/// </remarks>
struct AppConfig {
    AppRuntimeConfig app_runtime;
    SceneImportConfig scene_import;
    ScenePreprocessConfig scene_preprocess;
    MaterialConfig material;
    AntennaConfig antenna;
    PathSearchConfig path_search;
    EMSolverConfig em_solver;
    OutputConfig output;
    ValidationConfig validation;
    ExperimentConfig experiment;
    NumericToleranceConfig numeric_tolerance;
};

/// <summary>
/// 配置校验结果。
/// </summary>
/// <remarks>
/// 用于统一承载字段级、块内和跨块校验的告警与错误。
/// </remarks>
struct ConfigValidationResult {
    bool passed = true;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

} // namespace rt
