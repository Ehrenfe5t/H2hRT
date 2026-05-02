
#include"DxQReceiverAntenna.h"

namespace ReceiverAntennaStd {


	ReceiverAntenna::ReceiverAntenna()
	{
		this->receiverAntennaId = -1;
	}

	ReceiverAntenna::ReceiverAntenna(
		int receiverAntennaId, const std::vector<int>& transmittingAntennaIds, 
		const AntennaPropertyStd::AntennaProperty& antennaProperty)
	{
		this->receiverAntennaId = receiverAntennaId;
		this->antennaProperty = antennaProperty;
		this->transmittingAntennaIds.clear();
		for (int i = 0; i < transmittingAntennaIds.size(); ++i) {
			this->transmittingAntennaIds.emplace_back(transmittingAntennaIds[i]);
		}
	}

	ReceiverAntenna::ReceiverAntenna(
		int receiverAntennaId, const std::vector<int>& transmittingAntennaIds, 
		const Point3DStd::Point3D& location, double frequency, 
		int polarization3DModelId, int radiationPatternId)
	{
		this->receiverAntennaId = receiverAntennaId;
		this->antennaProperty.location = location;
		this->antennaProperty.frequencys.emplace_back(frequency);
		this->antennaProperty.polarization3DModelId = polarization3DModelId;
		this->antennaProperty.radiationPatternId = radiationPatternId;
		this->transmittingAntennaIds.clear();
		for (int i = 0; i < transmittingAntennaIds.size(); ++i) {
			this->transmittingAntennaIds.emplace_back(transmittingAntennaIds[i]);
		}
	}

	ReceiverAntenna::~ReceiverAntenna()
	{
	}

}