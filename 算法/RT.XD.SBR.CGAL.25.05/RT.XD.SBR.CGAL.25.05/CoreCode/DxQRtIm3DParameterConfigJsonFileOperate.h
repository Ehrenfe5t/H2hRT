#pragma once

#include"DxQRtIm3DParameterConfig.h"

namespace RtIm3DParameterConfigStd {

    std::string RtIm3DParameterConfigToJsonString(const RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DParameterConfigByJsonFile(const char* path, RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DParameterConfigToJsonFile(const char* path, const RtIm3DParameterConfigStd::RtIm3DParameterConfig& rtIm3DParameterConfig);


}