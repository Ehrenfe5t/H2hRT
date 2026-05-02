
#include"DxQTransmittingAntenna.h"

namespace TransmittingAntennaStd {


    TransmittingAntenna::TransmittingAntenna()
    {
        this->transmittingAntennaId = -1;
        this->emissionPower = 0.0;

    }


    TransmittingAntenna::TransmittingAntenna(
        int transmittingAntennaId, double emissionPower, const AntennaPropertyStd::AntennaProperty& antennaProperty)
    {
        this->transmittingAntennaId = transmittingAntennaId;
        this->emissionPower = emissionPower;
        this->antennaProperty = antennaProperty;
    }


    TransmittingAntenna::TransmittingAntenna(
        int transmittingAntennaId, double emissionPower, 
        const Point3DStd::Point3D& location, long long frequency, int polarization3DModelId, int radiationPatternId)
    {
        this->transmittingAntennaId = transmittingAntennaId;
        this->emissionPower = emissionPower;
        this->antennaProperty.location = location;
        this->antennaProperty.frequencys.emplace_back(frequency);
        this->antennaProperty.polarization3DModelId = polarization3DModelId;
        this->antennaProperty.radiationPatternId = radiationPatternId;
    }


    TransmittingAntenna::~TransmittingAntenna()
    {
    }


}