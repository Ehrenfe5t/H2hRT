#pragma once

#include"DxQRtIm3DPrivateParameterConfig.h"

#include"QzQJson.hpp"
namespace RtIm3DPrivateParameterConfigStd {

    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtIm3DPrivateParameterConfigStd::RtIm3DPrivateParameterConfig& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtIm3DPrivateParameterConfigStd::RtIm3DPrivateParameterConfig& obj);

}