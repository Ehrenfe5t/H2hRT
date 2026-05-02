#pragma once


#include"HdQAntennaProperty.h"

namespace ReceiverAntennaStd {

	class ReceiverAntenna
	{
	public:

		int receiverAntennaId;

		std::vector<int> transmittingAntennaIds{};

		AntennaPropertyStd::AntennaProperty antennaProperty;

		ReceiverAntenna();
		ReceiverAntenna(int receiverAntennaId, const std::vector<int>& transmittingAntennaIds,
			const AntennaPropertyStd::AntennaProperty& antennaProperty);
		ReceiverAntenna(int receiverAntennaId, const std::vector<int>& transmittingAntennaIds,
			const Point3DStd::Point3D& location, double frequency,
			int polarization3DModelId, int radiationPatternId);
		~ReceiverAntenna();

	private:

	};

}