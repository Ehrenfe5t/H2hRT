

#include"LxQPoint3I.h"
namespace Point3IStd {


	Point3I::Point3I()
	{
		this->x = -1;
		this->y = -1;
		this->z = -1;
	}

	Point3I::Point3I(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Point3I::~Point3I()
	{
	}

}