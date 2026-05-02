
#include"DxQRtSbr3DForRay3DPrivateParameterConfig.h"

namespace RtSbr3DForRay3DPrivateParameterConfigStd {

	RtSbr3DForRay3DPrivateParameterConfig::RtSbr3DForRay3DPrivateParameterConfig()
	{
		this->realWorldRefraction = true;
		this->cylindricalTube = false;
		this->radiusCorner = 0.03;
		this->radiusRx = 0.05;
		this->rayNumber = 10000;
		this->gapDiffractionRad = 0.05;
		this->gapDiffuseScatteringAzimuth = 0.05;
		this->gapDiffuseScatteringPitchAngle = 0.05;
	}

	RtSbr3DForRay3DPrivateParameterConfig::~RtSbr3DForRay3DPrivateParameterConfig()
	{
	}

}