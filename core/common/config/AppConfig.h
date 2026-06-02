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
    // v6: worker_threads removed (OpenMP auto-detects)
};

/// <summary>
/// 场景导入配置。
/// </summary>
struct SceneImportConfig {
    std::string source_file;
    std::string source_format = "obj";
    std::string coordinate_transform = "none";  // "none" | "blender_z_up_to_y_up"
    std::string scene_material_map_file = "configs/scenes/scene_material_map.json";
    bool normalize_object_names = true;
    // v6: allow_name_auto_cleanup removed
};

/// <summary>
/// 场景预处理配置。
/// </summary>
struct ScenePreprocessConfig {
    bool rebuild_normals = false;
    bool enable_wedge_build = true;
    bool enable_scene_cache = false;
    // v6: removed enable_bvh_bruteforce, filter_non_manifold, skip_coplanar, wedge_angles, preprocess_mode, cache_versions, bvh_bruteforce_sample
    int bvh_leaf_size = 16;
};

/// <summary>
/// 材料系统配置。
/// </summary>
struct MaterialConfig {
    std::string material_database_file;
    // v9 step28: 材质策略 — strict=缺失中止, warn=warning+真空fallback, debug=忽略
    std::string missing_material_policy = "strict";
    // v6: material_mapping_file, frequency_query_mode, allow_material_fallback, default_background_medium removed
    // 材质映射统一用 scene_import.scene_material_map_file
};

/// <summary>
/// 天线输入配置。
/// v8: 新增天线姿态 (forward/up) 和逐角度极化方向图支持。
/// </summary>
struct AntennaConfig {
    std::string source_type = "Ideal";
    std::string pattern_file;          // 增益方向图CSV
    std::string polarization_file;     // v7: 固定极化向量; v8: 逐角度极化CSV (含PolTheta/PolPhi)
    // v8: 天线姿态 — 世界坐标中的天线方向
    double forward_x = 1.0, forward_y = 0.0, forward_z = 0.0;  // 天线主瓣指向 (boresight)
    double up_x = 0.0, up_y = 0.0, up_z = 1.0;                // 天线"上"方向 (确定极化参考)
};

/// <summary>
/// v8: 多 Rx 目标位置。
/// </summary>
struct RxTarget {
    std::string id;
    double x = 0.0, y = 0.0, z = 0.0;
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
    bool enable_los = true;
    int max_consecutive_same_interaction = 5;
    // v9 step11: 剪枝阈值配置化 (原硬编码8/16)
    int per_expander_keep_limit = 8;     // 每个扩展器保留候选数
    int per_state_keep_limit = 16;       // 每个状态保留候选总数
    // v9 step11: wedge候选配置化 (原硬编码50m/64)
    double wedge_max_distance_m = 50.0;  // 绕射候选最大距离
    int wedge_max_candidates = 64;       // 绕射候选最大数量
    // v9 step28: 物理容忍度配置
    double direction_closure_angle_tol_deg = 2.0; // LS闭合方向容差(°) 原硬编码2°
    double snell_residual_tol = 1.0e-6;           // Snell残差阈值
    bool exhaustive_debug_mode = false;            // 关闭top-K剪枝, 用于对照实验
    // v9 D-3: 绕射/散射控制
    bool allow_boundary_edge_diffraction = true;     // 边界边是否参与绕射
    bool enable_lambertian_scattering = false;       // Lambertian漫散射 (实验性)
    double scattering_coefficient = 0.0;             // 漫散射系数 [0,1]
    // v10 Iter5: SBR辅助候选池
    bool enable_sbr_auxiliary_pool = false;          // 启用SBR辅助候选池
    int sbr_auxiliary_ray_count = 5000;              // 辅助SBR射线数(默认5000)
    double tx_x = 1.0, tx_y = 1.0, tx_z = 1.0;
    double rx_x = 3.0, rx_y = 1.0, rx_z = 1.0;
    // v8: multi-Rx list (if non-empty, overrides single rx_x/y/z)
    std::vector<RxTarget> rx_list;
};

/// <summary>
/// SBR 覆盖仿真配置。
/// </summary>
struct SbrConfig {
    bool enabled = false;
    int ray_count = 10000;
    int max_ray_depth = 6;
    int max_reflection_count = 6;
    // v7.5: SBR enable_*移除, 统一max_*_count=0控制
    int max_transmission_count = 0;
    int max_diffraction_count = 0;
    double ray_power_threshold_dB = -60.0;  // v6: was linear, now dB relative to Tx
    double rx_sphere_radius_m = 0.3;
    bool auto_grid_bounds = true;
    // v6: grid_margin_m removed
    double rx_grid_min_x = -5.0;
    double rx_grid_max_x = 5.0;
    double rx_grid_min_y = -5.0;
    double rx_grid_max_y = 5.0;
    double rx_grid_min_z = 1.5;
    double rx_grid_max_z = 1.5;
    double rx_grid_step_x = 1.0;
    double rx_grid_step_y = 1.0;
    double rx_grid_step_z = 1.0;
    double tx_power_dBm = 0.0;  // v6: was tx_power_w (W), now dBm. 0dBm=1mW
    bool store_paths = false;
    double wedge_max_distance_m = 5.0;
    int wedge_max_candidates = 8;  // v6: was wedge_sample_count
};

/// <summary>
/// 电磁求解配置。
/// </summary>
struct EMSolverConfig {
    double frequency_hz = 2.4e9;
    std::string solver_mode = "Precise";
    // v6: enable_polarization removed
};

/// <summary>
/// v9 主线C: 频率扫频配置 — 宽带信道仿真
/// </summary>
struct FrequencySweepConfig {
    bool enabled = false;             // 是否启用频域扫频
    double center_hz = 3.0e9;        // 中心频率 (Hz)
    double bandwidth_hz = 2.0e8;     // 带宽 (Hz)
    int point_count = 201;           // 频点数
    std::string spacing = "linear";  // "linear" | "log"
    bool retrace_per_frequency = false; // 每频点重新几何寻径 (默认复用路径)
    std::string mode = "fixed_gain"; // "fixed_gain" | "frequency_sweep_em"
};

/// <summary>
/// v9 主线C: 信道观测配置 — 测量等效CIR参数
/// </summary>
struct ChannelObservationConfig {
    bool export_ideal_delta_cir = true;   // 理想δ CIR
    bool export_sampled_cfr = true;       // 采样CFR H(f)
    bool export_observed_cir_ifft = true; // IFFT可观测CIR
    double delay_bin_s = 1.0e-9;          // 时延分bin宽度
    std::string window_type = "hann";     // 窗函数: "hann"|"rect"|"hamming"
    std::string ifft_convention = "vna_like"; // IFFT约定
};

/// <summary>
/// 输出控制配置。
/// </summary>
struct OutputConfig {
    bool export_paths = true;
    bool export_cir = false;
    bool export_pdp = false;
    bool export_aps = false;
    bool export_config_snapshot = true;
    // v6: output_directory, export_debug_files removed
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
/// v8: 管线阶段控制配置 — 控制混合加速管线的各阶段开关
/// </summary>
struct PipelineConfig {
    bool enable_stage0_precompute = true;
    bool enable_pvs = true;
    bool enable_edge_adjacency = true;
    bool enable_angular_grid = true;
    bool enable_stage1_coarse_sbr = false;     // Phase 2实现
    bool enable_stage2_constrained_search = false; // Phase 3实现
    bool enable_stage3_path_reuse = false;     // Phase 4实现
    bool enable_stage4_precise_em = true;
    bool enable_legacy_sbr_power = true;
    int seed_rx_count = 2000;
    double seed_spatial_stride = 0.5;
    int bidirectional_split_depth = 5;
    int max_paths_per_rx = 256;
    double reuse_max_distance = 2.0;
    bool reuse_verify_last_hop = true;
};

/// <summary>
/// v8: 加速器后端配置
/// </summary>
struct AccelerationConfig {
    std::string backend = "CPU_FaceBVH";
    int gpu_device_id = 0;
    int gpu_batch_size = 1000000;
    bool gpu_use_rt_core = true;
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
    SbrConfig sbr;
    EMSolverConfig em_solver;
    OutputConfig output;
    ValidationConfig validation;
    ExperimentConfig experiment;
    PipelineConfig pipeline;            // v8: 管线阶段控制
    AccelerationConfig acceleration;    // v8: 加速器后端选择
    NumericToleranceConfig numeric_tolerance;
    FrequencySweepConfig frequency_sweep;       // v9 C: 宽带扫频
    ChannelObservationConfig channel_observation; // v9 C: 信道观测
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
