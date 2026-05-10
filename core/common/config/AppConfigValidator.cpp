// v7 H9: 实现基础AppConfig验证 — 检查频率/深度/容差/交互开关一致性
#include "AppConfigValidator.h"
#include "AppConfig.h"
#include <string>

namespace rt {

ConfigValidationResult ValidateAppConfig(const AppConfig& config)
{
    ConfigValidationResult r;
    std::vector<std::string>& e = r.errors;
    r.passed = true;

    auto fail = [&](const char* msg) { r.passed = false; e.push_back(msg); };

    // 频率
    if (config.em_solver.frequency_hz <= 0.0)
        fail("em_solver.frequency_hz must be > 0");

    // 路径搜索深度和交互计数
    if (config.path_search.max_path_depth < 1)
        fail("path_search.max_path_depth must be >= 1");
    if (config.path_search.max_reflection_count < 0)
        fail("path_search.max_reflection_count must be >= 0");
    if (config.path_search.max_transmission_count < 0)
        fail("path_search.max_transmission_count must be >= 0");
    if (config.path_search.max_diffraction_count < 0)
        fail("path_search.max_diffraction_count must be >= 0");

    // 交互开关与计数一致性: 计数=0等价于开关关闭
    if (!config.path_search.enable_transmission && config.path_search.max_transmission_count > 0)
        fail("path_search.enable_transmission=false but max_transmission_count>0");
    if (!config.path_search.enable_diffraction && config.path_search.max_diffraction_count > 0)
        fail("path_search.enable_diffraction=false but max_diffraction_count>0");

    // SBR
    if (config.sbr.enabled) {
        if (config.sbr.ray_count <= 0) fail("sbr.ray_count must be > 0");
        if (config.sbr.max_ray_depth < 1) fail("sbr.max_ray_depth must be >= 1");
        if (config.sbr.rx_sphere_radius_m <= 0.0) fail("sbr.rx_sphere_radius_m must be > 0");
        if (config.sbr.enable_transmission && config.sbr.max_transmission_count < 1)
            fail("sbr: enable_transmission but max_transmission_count=0");
        if (config.sbr.enable_diffraction && config.sbr.max_diffraction_count < 1)
            fail("sbr: enable_diffraction but max_diffraction_count=0");
    }

    // 容差
    if (config.numeric_tolerance.eps_length <= 0.0)
        fail("eps_length must be > 0");
    if (config.numeric_tolerance.eps_intersection <= 0.0)
        fail("eps_intersection must be > 0");

    // 场景文件
    if (config.scene_import.source_file.empty())
        fail("scene_import.source_file is empty");
    if (config.scene_import.scene_material_map_file.empty())
        fail("scene_import.scene_material_map_file is empty");

    return r;
}

bool IsSupportedPreprocessMode(const std::string& mode) {
    return mode == "full" || mode == "cache_only" || mode == "build_only";
}
bool IsSupportedFrequencyQueryMode(const std::string& mode) {
    return mode == "interpolate" || mode == "exact" || mode == "nearest";
}
}
