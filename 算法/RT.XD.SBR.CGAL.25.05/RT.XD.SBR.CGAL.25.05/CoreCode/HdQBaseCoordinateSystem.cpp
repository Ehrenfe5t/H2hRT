



#include"HdQBaseCoordinateSystem.h"

namespace BaseCoordinateSystemStd {

	BaseCoordinateSystem::BaseCoordinateSystem(const Point3DStd::Point3D& o, const Point3DStd::Point3D& x, const Point3DStd::Point3D& y, const Point3DStd::Point3D& z)
	{
		this->o = o;
		this->x = x;
		this->y = y;
		this->z = z;
	}

	BaseCoordinateSystem::BaseCoordinateSystem()
	{
	}

	BaseCoordinateSystem::~BaseCoordinateSystem()
	{
	}
}