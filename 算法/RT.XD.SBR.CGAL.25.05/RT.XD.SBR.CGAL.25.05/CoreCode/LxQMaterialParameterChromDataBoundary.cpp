

#include"LxQMaterialParameterChromDataBoundary.h"


namespace MaterialParameterChromDataBoundaryStd {

	MaterialParameterChromDataBoundary::MaterialParameterChromDataBoundary()
	{
		this->conductivityMin = 0;
		this->conductivityMax = 10000;
		this->relativePermittivityMin = 1;
		this->relativePermittivityMax = 200;
	}

	MaterialParameterChromDataBoundary::~MaterialParameterChromDataBoundary()
	{
	}


}