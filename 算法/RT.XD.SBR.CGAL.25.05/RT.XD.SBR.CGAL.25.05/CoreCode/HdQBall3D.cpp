



#include"HdQBall3D.h"

namespace Ball3DStd {


	Ball3D::Ball3D(double radius, const Point3DStd::Point3D& center)
	{
		this->radius = radius;
		this->center = center;
	}

	Ball3D::Ball3D()
	{
		this->radius = 0.0;
	}

	Ball3D::~Ball3D()
	{
	}

	bool Ball3D::IsPoint3DInBox(const Point3DStd::Point3D& obj)
	{
		double dx = obj.x - center.x;
		double dy = obj.y - center.y;
		double dz = obj.z - center.z;
		double dis = sqrt(dx * dx + dy * dy + dz * dz);
		return dis<=radius;
	}

	bool Ball3D::IsTriangle3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3)
	{
		return IsPoint3DInBox(p1) && IsPoint3DInBox(p2) && IsPoint3DInBox(p3);
	}

	bool Ball3D::IsLineSegment3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
	{
		return IsPoint3DInBox(p1) && IsPoint3DInBox(p2);
	}

	bool Ball3D::IsColliding(const Ball3D& ball)
	{

		double dx = this->center.x - ball.center.x;
		double dy = this->center.y - ball.center.y;
		double dz = this->center.z - ball.center.z;

		double dis = sqrt(dx * dx + dy * dy + dz * dz);

		return dis<this->radius+ball.radius;
	}


}