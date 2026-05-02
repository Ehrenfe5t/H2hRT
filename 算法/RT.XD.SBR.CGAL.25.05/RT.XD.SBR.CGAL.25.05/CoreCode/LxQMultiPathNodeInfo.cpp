

#include "LxQMultiPathNodeInfo.h"


namespace MultiPathNodeInfoStd {


	MultiPathNodeInfo::MultiPathNodeInfo(PropagationTypeStd::PropagationType type, const Point3DStd::Point3D& location)
	{
		this->type = type;
		this->location = location;
	}

	MultiPathNodeInfo::MultiPathNodeInfo()
	{
		this->type = PropagationTypeStd::PropagationType::Null;
	}

	MultiPathNodeInfo::~MultiPathNodeInfo()
	{
	}

}


