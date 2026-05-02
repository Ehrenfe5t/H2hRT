
#include"LxQMultiPathNodeInfoTransmission.h"

namespace MultiPathNodeInfoStd {
	MultiPathNodeInfoTransmission::MultiPathNodeInfoTransmission(
		int upObjectType,
		int downObjectType,
		double thetai,
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& normalVector)
	{
		this->type = PropagationTypeStd::PropagationType::Transmission;
		this->location = location;
		this->normalVector = normalVector;

		this->upObjectType = upObjectType;
		this->downObjectType = downObjectType;
		this->thetai = thetai;
	}
	MultiPathNodeInfoTransmission::MultiPathNodeInfoTransmission()
	{
		this->type = PropagationTypeStd::PropagationType::Transmission;

		this->upObjectType = -999;
		this->downObjectType = -999;
		this->thetai = 0.0;
	}
	MultiPathNodeInfoTransmission::~MultiPathNodeInfoTransmission()
	{
	}
}