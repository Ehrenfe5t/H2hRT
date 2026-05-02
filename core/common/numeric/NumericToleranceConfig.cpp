// 文件目标：
// - 实现统一数值容差配置的合法性校验。
//
// 主要功能：
// - 检查所有容差字段是否为正值；
// - 以 ConfigValidationResult 形式返回错误，供 AppConfigValidator 汇总。

#include "NumericToleranceConfig.h"

#include "../config/AppConfig.h"

namespace rt {

/// <summary>
/// 校验统一数值容差配置。
/// </summary>
/// <returns>包含所有非法容差字段错误信息的配置校验结果。</returns>
ConfigValidationResult NumericToleranceConfig::Validate() const
{
    ConfigValidationResult result;

    if (eps_length <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_length must be > 0.");
    }
    if (eps_angle <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_angle must be > 0.");
    }
    if (eps_intersection <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_intersection must be > 0.");
    }
    if (eps_normal <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_normal must be > 0.");
    }
    if (eps_deduplicate <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_deduplicate must be > 0.");
    }
    if (eps_power <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.eps_power must be > 0.");
    }
    if (self_hit_ignore_distance <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.self_hit_ignore_distance must be > 0.");
    }
    if (visibility_origin_offset <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.visibility_origin_offset must be > 0.");
    }
    if (visibility_target_shrink <= 0.0)
    {
        result.passed = false;
        result.errors.push_back("numeric_tolerance.visibility_target_shrink must be > 0.");
    }

    return result;
}

} // namespace rt
