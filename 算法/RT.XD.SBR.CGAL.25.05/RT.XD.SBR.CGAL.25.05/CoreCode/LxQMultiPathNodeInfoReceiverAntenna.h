#pragma once

#include"LxQMultiPathNodeInfo.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfoReceiverAntenna : public MultiPathNodeInfo
	{
	public:

		/// <summary>
		/// ÌìÏßid
		/// </summary>
		int antennaID;


		MultiPathNodeInfoReceiverAntenna(
			int antennaID, 
			const Point3DStd::Point3D& location);
		MultiPathNodeInfoReceiverAntenna();
		~MultiPathNodeInfoReceiverAntenna();

	private:

	};

}
