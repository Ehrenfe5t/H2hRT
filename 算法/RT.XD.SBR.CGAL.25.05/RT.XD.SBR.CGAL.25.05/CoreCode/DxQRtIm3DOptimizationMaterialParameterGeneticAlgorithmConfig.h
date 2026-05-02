#pragma once


#include"DxQRtIm3DParameterConfig.h"
#include"LxQOptimizationMaterialParameterGeneticAlgorithmConfig.h"
namespace RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfigStd {

	class RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig
	{
	public:
		RtIm3DParameterConfigStd::RtIm3DParameterConfig rtIm3DParameterConfig;
		OptimizationMaterialParameterGeneticAlgorithmConfigStd::OptimizationMaterialParameterGeneticAlgorithmConfig optimizationMaterialParameterGeneticAlgorithmConfig;
		RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig();
		~RtIm3DOptimizationMaterialParameterGeneticAlgorithmConfig();

	private:

	};


}
