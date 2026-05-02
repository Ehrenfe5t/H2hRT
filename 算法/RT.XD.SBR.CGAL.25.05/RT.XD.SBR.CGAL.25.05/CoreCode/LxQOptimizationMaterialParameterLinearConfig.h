#pragma once

#include"LxQOptimizationMaterialParameterConfig.h"

namespace OptimizationMaterialParameterLinearConfigStd {


	class OptimizationMaterialParameterLinearConfig
	{
	public:

		/// <summary>
		/// 最大搜索次数
		/// </summary>
		int searchMaxNumber;

		/// <summary>
		/// 搜索目标
		/// </summary>
		double searchRmseGoal;

		/// <summary>
		/// 搜索间隔
		/// </summary>
		double searchGap;

		/// <summary>
		/// 左右间隔
		/// </summary>
		double left_right_eps;


		OptimizationMaterialParameterConfigStd::OptimizationMaterialParameterConfig optimizationMaterialParameterConfig;

		OptimizationMaterialParameterLinearConfig();
		~OptimizationMaterialParameterLinearConfig();

	private:

	};


}