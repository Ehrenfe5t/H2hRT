

#include"LxQOptimizationMaterialParameterLinearConfig.h"

namespace OptimizationMaterialParameterLinearConfigStd {


	OptimizationMaterialParameterLinearConfig::OptimizationMaterialParameterLinearConfig()
	{

		this->searchMaxNumber = 200;
		this->searchRmseGoal = 5.1;
		this->searchGap = 0.001;
		this->left_right_eps = 1e-4;
	}

	OptimizationMaterialParameterLinearConfig::~OptimizationMaterialParameterLinearConfig()
	{
	}


}