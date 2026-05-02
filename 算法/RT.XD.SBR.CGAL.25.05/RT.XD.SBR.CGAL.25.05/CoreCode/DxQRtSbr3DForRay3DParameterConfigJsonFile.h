#pragma once

#include"DxQRtSbr3DForRay3DParameterConfig.h"

namespace RtSbr3DForRay3DParameterConfigStd {


    std::string RtSbr3DForRay3DParameterConfigToJsonString(const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtSbr3DForRay3DParameterConfigByJsonFile(const char* path, RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtSbr3DForRay3DParameterConfigToJsonFile(const char* path, const RtSbr3DForRay3DParameterConfigStd::RtSbr3DForRay3DParameterConfig& rtSbr3DForRay3DParameterConfig);

}