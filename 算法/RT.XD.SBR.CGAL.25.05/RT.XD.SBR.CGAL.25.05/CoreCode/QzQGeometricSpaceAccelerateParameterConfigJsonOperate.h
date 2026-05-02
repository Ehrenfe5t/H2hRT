#pragma once

#include"QzQGeometricSpaceAccelerateParameterConfig.h"

#include"QzQJson.hpp"

namespace GeometricSpaceAccelerateParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const GeometricSpaceAccelerateParameterConfigStd::GeometricSpaceAccelerateParameterConfig& obj);



}