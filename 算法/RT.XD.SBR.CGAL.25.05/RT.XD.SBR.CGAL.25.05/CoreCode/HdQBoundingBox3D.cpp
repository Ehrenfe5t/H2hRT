
#include"HdQBoundingBox3D.h"
#include <limits>
namespace BoundingBox3DStd {


	BoundingBox3D::BoundingBox3D()
	{
		this->min.x = std::numeric_limits<double>::max();;
		this->min.y = std::numeric_limits<double>::max();;
		this->min.z = std::numeric_limits<double>::max();;
		this->max.x = -std::numeric_limits<double>::max();
		this->max.y = -std::numeric_limits<double>::max();
		this->max.z = -std::numeric_limits<double>::max();
	}


	BoundingBox3D::BoundingBox3D(const Point3DStd::Point3D& min, const Point3DStd::Point3D& max)
	{
		this->min.x = min.x;
		this->min.y = min.y;
		this->min.z = min.z;
		this->max.x = max.x;
		this->max.y = max.y;
		this->max.z = max.z;
	}


	BoundingBox3D::BoundingBox3D(const BoundingBox3D& obj)
	{
		this->min.x = obj.min.x;
		this->min.y = obj.min.y;
		this->min.z = obj.min.z;
		this->max.x = obj.max.x;
		this->max.y = obj.max.y;
		this->max.z = obj.max.z;
	}

	BoundingBox3D::~BoundingBox3D()
	{
	}

	bool BoundingBox3D::IsXInBox(double x)
	{
		return x >= min.x && x <= max.x;
	}

	bool BoundingBox3D::IsYInBox(double y)
	{
		return y >= min.y && y <= max.y;
	}

	bool BoundingBox3D::IsZInBox(double z)
	{
		return z >= min.z && z <= max.z;
	}

	bool BoundingBox3D::IsPoint3DInBox(const Point3DStd::Point3D& obj)
	{
		return IsXInBox(obj.x) && IsYInBox(obj.y) && IsZInBox(obj.z);
	}

	bool BoundingBox3D::IsColliding(const BoundingBox3DStd::BoundingBox3D& box) {
		return (this->min.x <= box.max.x && this->max.x >= box.min.x)
			&& (this->min.y <= box.max.y && this->max.y >= box.min.y)
			&& (this->min.z <= box.max.z && this->max.z >= box.min.z);
	}
}