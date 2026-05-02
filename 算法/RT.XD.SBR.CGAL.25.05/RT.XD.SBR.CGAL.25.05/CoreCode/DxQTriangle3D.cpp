

#include"DxQTriangle3D.h"


namespace Triangle3DStd {
	

	Triangle3D::Triangle3D()
	{
	}

	Triangle3D::Triangle3D(const Triangle3DStd::Triangle3D& obj) {
		this->p1.x = obj.p1.x;
		this->p1.y = obj.p1.y;
		this->p1.z = obj.p1.z;

		this->p2.x = obj.p2.x;
		this->p2.y = obj.p2.y;
		this->p2.z = obj.p2.z;

		this->p3.x = obj.p3.x;
		this->p3.y = obj.p3.y;
		this->p3.z = obj.p3.z;
	}
	Triangle3D::Triangle3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3) {
		this->p1.x = p1.x;
		this->p1.y = p1.y;
		this->p1.z = p1.z;

		this->p2.x = p2.x;
		this->p2.y = p2.y;
		this->p2.z = p2.z;

		this->p3.x = p3.x;
		this->p3.y = p3.y;
		this->p3.z = p3.z;
	}

	Triangle3D::~Triangle3D()
	{
	}

}


