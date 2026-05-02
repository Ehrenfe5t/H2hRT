#pragma once

#include"DxQRtIm3DOptimizationMaterialParameterLinearConfig.h"


namespace RtIm3DOptimizationMaterialParameterLinearConfigStd {

    std::string RtIm3DOptimizationMaterialParameterLinearConfigToJsonString(const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DOptimizationMaterialParameterLinearConfigByJsonFile(const char* path, RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DOptimizationMaterialParameterLinearConfigToJsonFile(const char* path, const RtIm3DOptimizationMaterialParameterLinearConfigStd::RtIm3DOptimizationMaterialParameterLinearConfig& rtIm3DOptimizationMaterialParameterLinearConfig);


}