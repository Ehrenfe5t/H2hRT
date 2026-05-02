

#include"LxQMultiPathNodeInfoReceiverAntenna.h"

namespace MultiPathNodeInfoStd {


	MultiPathNodeInfoReceiverAntenna::MultiPathNodeInfoReceiverAntenna(
		int antennaID, 
		const Point3DStd::Point3D& location)
	{
		this->type = PropagationTypeStd::PropagationType::ReceiverAntenna;
		this->antennaID = antennaID;
		this->location = location;
	}

	MultiPathNodeInfoReceiverAntenna::MultiPathNodeInfoReceiverAntenna()
	{
		this->type = PropagationTypeStd::PropagationType::ReceiverAntenna;
		this->antennaID = -1;
	}

	MultiPathNodeInfoReceiverAntenna::~MultiPathNodeInfoReceiverAntenna()
	{
	}


}