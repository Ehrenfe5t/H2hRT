#pragma once

#include"DxQRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig.h"

namespace RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd {


    std::string RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigToJsonString(const RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);

    /// <summary>
    /// 读取配置文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    bool ReadRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigByJsonFile(const char* path, RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);


    /// <summary>
    /// 写入到json文件
    /// </summary>
    /// <param name="path"></param>
    /// <param name="object"></param>
    void WriteRtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigToJsonFile(const char* path, const RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd::RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig& rtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig);

}

