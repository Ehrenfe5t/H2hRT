#pragma once

#include"DxQRtSbr3DForRay3DParameterConfig.h"

#include"QzQJson.hpp"

namespace RtSbr3DForRay3DParameterConfigStd {


    /// <summary>
    /// 从json字符串获取点
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void from_json(const nlohmann::json& j, RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& obj);

    /// <summary>
    /// 将点对象转化为json字符串
    /// </summary>
    /// <param name="j"></param>
    /// <param name="p"></param>
    void to_json(nlohmann::json& j, const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& obj);

}