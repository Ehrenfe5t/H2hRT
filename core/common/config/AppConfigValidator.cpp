// v7 H9: 实现基础AppConfig验证 — 检查频率/深度/容差/交互开关一致性
// v10.2: 增加天线姿态验证、NumericToleranceConfig全量校验、Tx/Rx天线检查 (B9/W11修复)
#include "AppConfigValidator.h"
#include "AppConfig.h"
#include "../math/Vec3.h"
#include <string>
#include <cmath>

namespace rt {

ConfigValidationResult ValidateAppConfig(const AppConfig& config)
{
    ConfigValidationResult r;
    std::vector<std::string>& e = r.errors;
    r.passed = true;

    auto fail = [&](const char* msg) { r.passed = false; e.push_back(msg); };
    auto warn = [&](const char* msg) { r.warnings.push_back(msg); };

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

    // SBR
    const bool preciseP2PEnabled = config.pipeline.enable_stage4_precise_em
        && config.em_solver.solver_mode != "Coverage";
    if (preciseP2PEnabled) {
        if (config.sbr.ray_count <= 0) fail("sbr.ray_count must be > 0");
        if (config.sbr.max_ray_depth < 1) fail("sbr.max_ray_depth must be >= 1");
        if (config.sbr.rx_sphere_radius_m <= 0.0) fail("sbr.rx_sphere_radius_m must be > 0");
    }
    if (config.sbr.enabled) {
        if (config.sbr.rx_grid_step_x <= 0.0) fail("sbr.rx_grid_step_x must be > 0");
        if (config.sbr.rx_grid_step_y <= 0.0) fail("sbr.rx_grid_step_y must be > 0");
        if (config.sbr.rx_grid_step_z <= 0.0) fail("sbr.rx_grid_step_z must be > 0");
    }

    if (!preciseP2PEnabled)
        fail("v11 main chain requires pipeline.enable_stage4_precise_em=true and em_solver.solver_mode != Coverage");

    // 场景文件
    if (config.scene_import.source_file.empty())
        fail("scene_import.source_file is empty");
    if (config.scene_import.scene_material_map_file.empty())
        fail("scene_import.scene_material_map_file is empty");

    // ── v10.2 W11修复: NumericToleranceConfig 全量校验 ──
    NumericToleranceConfig tolCfg = config.numeric_tolerance;
    if (tolCfg.eps_length <= 0.0)
        fail("eps_length must be > 0");
    if (tolCfg.eps_intersection <= 0.0)
        fail("eps_intersection must be > 0");
    if (tolCfg.eps_angle <= 0.0)
        fail("eps_angle must be > 0");
    if (tolCfg.eps_normal <= 0.0)
        fail("eps_normal must be > 0");
    if (tolCfg.eps_deduplicate <= 0.0)
        fail("eps_deduplicate must be > 0");
    if (tolCfg.eps_power <= 0.0)
        fail("eps_power must be > 0");
    if (tolCfg.self_hit_ignore_distance <= 0.0)
        fail("self_hit_ignore_distance must be > 0");
    if (tolCfg.visibility_origin_offset <= 0.0)
        fail("visibility_origin_offset must be > 0");
    if (tolCfg.visibility_target_shrink <= 0.0)
        fail("visibility_target_shrink must be > 0");

    // ── v10.2 B9修复: 天线姿态正交性检查 ──
    auto checkAntennaPose = [&](const AntennaConfig& antCfg, const char* blockName) {
        if (antCfg.source_type.empty()) return; // 跳过未配置的块
        double fx = antCfg.forward_x, fy = antCfg.forward_y, fz = antCfg.forward_z;
        double ux = antCfg.up_x, uy = antCfg.up_y, uz = antCfg.up_z;
        // 计算 forward × up 的长度 (应接近1.0如果正交且归一化)
        double cx = fy*uz - fz*uy;
        double cy = fz*ux - fx*uz;
        double cz = fx*uy - fy*ux;
        double crossLen = std::sqrt(cx*cx + cy*cy + cz*cz);
        double fLen = std::sqrt(fx*fx + fy*fy + fz*fz);
        double uLen = std::sqrt(ux*ux + uy*uy + uz*uz);
        if (fLen < 1e-9 || uLen < 1e-9) {
            fail((std::string(blockName) + ".forward/up is zero-length vector").c_str());
        } else if (crossLen / (fLen * uLen) < 1e-4) {
            // forward 与 up 近乎平行
            fail((std::string(blockName) + ".forward and .up are nearly parallel; cannot form valid antenna frame").c_str());
        }
    };
    checkAntennaPose(config.antenna, "antenna");
    checkAntennaPose(config.tx_antenna, "tx_antenna");
    checkAntennaPose(config.rx_antenna, "rx_antenna");

    // ── v10.2: APS 网格配置检查 ──
    if (config.em_solver.aps_theta_bins < 2) fail("aps_theta_bins must be >= 2");
    if (config.em_solver.aps_phi_bins < 4) fail("aps_phi_bins must be >= 4");

    return r;
}

} // namespace rt
