#pragma once


#include"LxQMultiPathNodeInfo.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfoTransmittingAntenna :public MultiPathNodeInfo
	{
	public:
		/// <summary>
		/// ÌìÏßid
		/// </summary>
		int baseStationAntennaID;

		MultiPathNodeInfoTransmittingAntenna(
			int baseStationAntennaID,
			const Point3DStd::Point3D& location);
		MultiPathNodeInfoTransmittingAntenna();
		~MultiPathNodeInfoTransmittingAntenna();

	private:

	};

}