#pragma once

#include"HdQAntennaProperty.h"

namespace TransmittingAntennaStd {

	class TransmittingAntenna
	{
	public:

		/// <summary>
		/// 发射机编号
		/// </summary>
		int transmittingAntennaId;

		/// <summary>
		/// 发射功率,单位是w
		/// </summary>
		double emissionPower;

		/// <summary>
		/// 基础属性
		/// </summary>
		AntennaPropertyStd::AntennaProperty antennaProperty;

		TransmittingAntenna();
		TransmittingAntenna(int transmittingAntennaId, double emissionPower,
			const AntennaPropertyStd::AntennaProperty& antennaProperty);
		TransmittingAntenna(int transmittingAntennaId, double emissionPower, 
			const Point3DStd::Point3D& location, long long frequency,
			int polarization3DModelId, int radiationPatternId);

		~TransmittingAntenna();

	private:

	};



}