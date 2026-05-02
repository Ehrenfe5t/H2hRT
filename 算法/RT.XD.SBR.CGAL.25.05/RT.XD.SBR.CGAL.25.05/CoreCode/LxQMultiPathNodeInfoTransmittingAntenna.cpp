


#include"LxQMultiPathNodeInfoTransmittingAntenna.h"

namespace MultiPathNodeInfoStd {


	MultiPathNodeInfoTransmittingAntenna::MultiPathNodeInfoTransmittingAntenna(
		int baseStationAntennaID, 
		const Point3DStd::Point3D& location)
	{
		this->type = PropagationTypeStd::PropagationType::TransmittingAntenna;
		this->baseStationAntennaID = baseStationAntennaID;
		this->location = location;
	}

	MultiPathNodeInfoTransmittingAntenna::MultiPathNodeInfoTransmittingAntenna()
	{
		this->type = PropagationTypeStd::PropagationType::TransmittingAntenna;
		this->baseStationAntennaID = -1;
	}

	MultiPathNodeInfoTransmittingAntenna::~MultiPathNodeInfoTransmittingAntenna()
	{
	}

}