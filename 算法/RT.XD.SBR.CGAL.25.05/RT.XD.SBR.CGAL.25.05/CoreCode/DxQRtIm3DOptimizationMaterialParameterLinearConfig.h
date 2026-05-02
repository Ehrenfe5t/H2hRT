#pragma once

#include"DxQRtIm3DParameterConfig.h"
#include"LxQOptimizationMaterialParameterLinearConfig.h"

namespace RtIm3DOptimizationMaterialParameterLinearConfigStd {

	class RtIm3DOptimizationMaterialParameterLinearConfig
	{
	public:

		RtIm3DParameterConfigStd::RtIm3DParameterConfig rtIm3DParameterConfig;
		OptimizationMaterialParameterLinearConfigStd::OptimizationMaterialParameterLinearConfig optimizationMaterialParameterLinearConfig;

		RtIm3DOptimizationMaterialParameterLinearConfig();
		~RtIm3DOptimizationMaterialParameterLinearConfig();

	private:

	};


}