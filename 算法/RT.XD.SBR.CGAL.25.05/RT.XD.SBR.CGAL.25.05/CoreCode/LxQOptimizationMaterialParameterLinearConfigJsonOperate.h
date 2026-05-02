#pragma once


#include"LxQOptimizationMaterialParameterLinearConfig.h"

#include"QzQJson.hpp"
namespace OptimizationMaterialParameterLinearConfigStd {
    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, OptimizationMaterialParameterLinearConfigStd::OptimizationMaterialParameterLinearConfig& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const OptimizationMaterialParameterLinearConfigStd::OptimizationMaterialParameterLinearConfig& obj);
}