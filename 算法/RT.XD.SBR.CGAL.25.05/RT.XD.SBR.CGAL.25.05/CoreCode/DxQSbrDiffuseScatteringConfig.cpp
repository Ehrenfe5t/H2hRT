
#include"DxQSbrDiffuseScatteringConfig.h"

namespace SbrDiffuseScatteringConfigStd {


	SbrDiffuseScatteringConfig::SbrDiffuseScatteringConfig()
	{
        this->maxDiscreteSideLength = 1;
        this->gapDiffuseScatteringAzimuth = 0.05;
        this->gapDiffuseScatteringPitchAngle = 0.05;
	}

	SbrDiffuseScatteringConfig::~SbrDiffuseScatteringConfig()
	{
	}

}