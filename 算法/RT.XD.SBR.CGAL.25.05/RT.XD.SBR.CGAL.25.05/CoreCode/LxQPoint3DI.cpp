

#include"LxQPoint3DI.h"
namespace Point3DIStd {


	Point3DI::Point3DI()
	{
		this->index = -1;
	}

	Point3DI::Point3DI(int index, const Point3DStd::Point3D& p)
	{
		this->index = index;
		this->p = p;
	}

	Point3DI::~Point3DI()
	{
	}

}