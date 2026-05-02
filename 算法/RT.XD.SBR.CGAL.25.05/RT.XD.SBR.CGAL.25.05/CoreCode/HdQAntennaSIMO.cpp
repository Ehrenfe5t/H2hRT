#include "HdQAntennaSIMO.h"

#include"DxQTransmittingAntennaDatabase.h"
#include"HdQAntennaPatternDatabase.h"


namespace AntennaSIMOStd {
	
	AntennaSIMO::AntennaSIMO()
	{
	}
	AntennaSIMO::~AntennaSIMO()
	{
	}
	AntennaSIMO::AntennaSIMO(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna, const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas)
	{
		this->transmittingAntenna = transmittingAntenna;
		this->receiverAntennas = receiverAntennas;
	}


	AntennaSIMOStd::AntennaSIMO InitAntennaSIMO(
		double emissionPower,
		long long frequency,
		int tx_polarization3DModelId,
		const Point3DStd::Point3D& tx_location,
		const std::vector<Point3DStd::Point3D>& rx_locations,
		const std::vector<std::vector<double>>& antennaPatternData)
	{
		int radiationPatternId = 0;
		//0表示全极化；1表示线极化；2表示左旋圆极化
		int polarization3DModelId = tx_polarization3DModelId;
		TransmittingAntennaStd::TransmittingAntenna transmittingAntenna(1000000, emissionPower, tx_location, frequency, polarization3DModelId, radiationPatternId);

		TransmittingAntennaDatabaseStd::Add(transmittingAntenna);

		std::vector<ReceiverAntennaStd::ReceiverAntenna> receiverAntennas(rx_locations.size());
		for (int i = 0; i < rx_locations.size(); ++i) {
			receiverAntennas[i].receiverAntennaId = i + 1000000;
			receiverAntennas[i].transmittingAntennaIds.emplace_back(transmittingAntenna.transmittingAntennaId);
			receiverAntennas[i].antennaProperty.frequencys.emplace_back(frequency);
			receiverAntennas[i].antennaProperty.location = rx_locations[i];
			receiverAntennas[i].antennaProperty.polarization3DModelId = -1;
			receiverAntennas[i].antennaProperty.radiationPatternId = -1;
		}
		if (antennaPatternData.size() == 360 && antennaPatternData[0].size() == 181) {
			AntennaPatternObjectStd::AntennaPatternObject antennaPatternObject;
			antennaPatternObject.radiationPatternId = radiationPatternId;

			for (int i = 0; i < 360; ++i)
			{
				for (int j = 0; j < 181; ++j)
				{
					antennaPatternObject.radiationPattern[i][j] = antennaPatternData[i][j];
				}
			}

			AntennaPatternDatabaseStd::AddAntennaPatternObject(antennaPatternObject);
		}

		return AntennaSIMOStd::AntennaSIMO(transmittingAntenna, receiverAntennas);
	}

}