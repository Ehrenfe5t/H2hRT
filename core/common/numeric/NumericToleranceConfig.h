// 文件目标：
// - 定义模块1统一数值容差配置对象。
//
// 主要功能：
// - 作为几何与电磁共用阈值的集中入口；
// - 为模块2/4/5后续实现避免 magic number 提供统一契约；
// - 提供自身合法性校验接口。

#pragma once

#include <string>
#include <vector>

namespace rt {

struct ConfigValidationResult;

/// <summary>
/// 统一数值容差配置。
/// </summary>
struct NumericToleranceConfig {
    double eps_length = 1.0e-6;
    double eps_angle = 1.0e-6;
    double eps_intersection = 1.0e-7;
    double eps_normal = 1.0e-6;
    double eps_deduplicate = 1.0e-5;
    double eps_power = 1.0e-9;
    double self_hit_ignore_distance = 1.0e-5;
    double visibility_origin_offset = 1.0e-5;
    double visibility_target_shrink = 1.0e-5;

    /// <summary>
    /// 校验当前容差配置是否合法。
    /// </summary>
    /// <returns>容差配置校验结果。</returns>
    ConfigValidationResult Validate() const;
};

} // namespace rt
