#include "HdQCommonParameterConfig.h"




namespace CommonParameterConfigStd {
	
	CommonParameterConfig::CommonParameterConfig()
	{
		this->airSubstanceType = -999;
		this->electricFieldCalculationMode = 1;
		this->energyOutputMode = 0;
		this->powerThreshold = -327;
		this->rebuildEdge = false;
		this->deduplicateRadius = 0.1;
	}

	CommonParameterConfig::~CommonParameterConfig()
	{
	}

}


