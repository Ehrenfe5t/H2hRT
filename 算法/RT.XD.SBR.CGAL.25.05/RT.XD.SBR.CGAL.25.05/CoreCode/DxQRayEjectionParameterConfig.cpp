#include "DxQRayEjectionParameterConfig.h"


namespace RayEjectionParameterConfigStd {
    
    
    RayEjectionParameterConfig::RayEjectionParameterConfig()
    {
        this->ejectionsMaxTotalNumber = 12;
        this->ejectionsOfDiffractionMaxNumber = 1;
        this->ejectionsOfDiffuseScatteringMaxNumber = 0;
        this->ejectionsOfReflectionMaxNumber = 2;
        this->ejectionsOfTransmissionMaxNumber = 2;
        this->switchOfLos = true;
    }

    RayEjectionParameterConfig::~RayEjectionParameterConfig()
    {
    }




}

