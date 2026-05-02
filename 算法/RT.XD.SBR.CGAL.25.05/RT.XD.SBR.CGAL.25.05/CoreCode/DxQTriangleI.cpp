

#include"DxQTriangleI.h"

namespace TriangleIStd {


	TriangleI::TriangleI()
	{
		index = -1;
	}

	TriangleI::~TriangleI()
	{
	}
	TriangleI::TriangleI(int index, const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2,
		const Point3DStd::Point3D& p3,
		const Point3DStd::Point3D& n)
	{
		this->index = index;
		this->p1 = p1;
		this->p2 = p2;
		this->p3 = p3;
		this->n = n;
	}

	Point3DStd::Point3D GetCenter(const TriangleIStd::TriangleI& obj)
	{
		double x = obj.p1.x + obj.p2.x + obj.p3.x;
		double y = obj.p1.y + obj.p2.y + obj.p3.y;
		double z = obj.p1.z + obj.p2.z + obj.p3.z;

		return Point3DStd::Point3D(x / 3.0, y / 3.0, z / 3.0);
	}

}