

#include"LxQMultiPathNodeInfoReflection.h"

namespace MultiPathNodeInfoStd {
	MultiPathNodeInfoReflection::MultiPathNodeInfoReflection(
		int upObjectType, 
		int downObjectType, 
		double thetai, 
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& normalVector)
	{
		this->type = PropagationTypeStd::PropagationType::Reflection;

		this->location = location;
		this->normalVector = normalVector;

		this->upObjectType = upObjectType;
		this->downObjectType = downObjectType;
		this->thetai = thetai;
	}
	MultiPathNodeInfoReflection::MultiPathNodeInfoReflection()
	{
		this->type = PropagationTypeStd::PropagationType::Reflection;
		this->upObjectType = -999;
		this->downObjectType = -999;
		this->thetai = 0.0;
	}
	MultiPathNodeInfoReflection::~MultiPathNodeInfoReflection()
	{
	}
}