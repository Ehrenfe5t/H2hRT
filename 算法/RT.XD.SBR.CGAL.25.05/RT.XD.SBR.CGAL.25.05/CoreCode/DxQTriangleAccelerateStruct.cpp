#include "DxQTriangleAccelerateStruct.h"

#include"QzQGlobalConstant.h"


namespace TriangleAccelerateStructStd {



	TriangleAccelerateStruct::TriangleAccelerateStruct()
	{
		this->upTypeNumber = GlobalConstantStd::GetAirSubstanceType();
		this->downTypeNumber = GlobalConstantStd::GetAirSubstanceType();
	}

	TriangleAccelerateStruct::~TriangleAccelerateStruct()
	{
	}

}