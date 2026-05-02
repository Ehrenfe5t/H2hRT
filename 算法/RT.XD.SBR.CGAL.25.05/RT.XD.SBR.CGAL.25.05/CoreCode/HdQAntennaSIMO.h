#pragma once
#include"DxQTransmittingAntenna.h"
#include"DxQReceiverAntenna.h"
#include<vector>

namespace AntennaSIMOStd {

	class AntennaSIMO
	{
	public:
		/// <summary>
		/// 发射天线
		/// </summary>
		TransmittingAntennaStd::TransmittingAntenna transmittingAntenna;

		/// <summary>
		/// 接收天线
		/// </summary>
		std::vector<ReceiverAntennaStd::ReceiverAntenna> receiverAntennas;
		AntennaSIMO();
		~AntennaSIMO();

		AntennaSIMO(
			const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna,
			const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas);

	private:

	};

	AntennaSIMOStd::AntennaSIMO InitAntennaSIMO(
		double emissionPower,
		long long frequency,
		int tx_polarization3DModelId,
		const Point3DStd::Point3D& tx_location,
		const std::vector<Point3DStd::Point3D>& rx_locations,
		const std::vector<std::vector<double>>& antennaPatternData);

}