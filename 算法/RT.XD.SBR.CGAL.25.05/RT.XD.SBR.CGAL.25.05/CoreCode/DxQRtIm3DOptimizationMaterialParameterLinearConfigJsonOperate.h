#pragma once

#include"DxQRtIm3DOptimizationMaterialParameterLinearConfig.h"

#include"QzQJson.hpp"
namespace RtIm3DOptimizationMaterialParameterLinearConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& obj);



}