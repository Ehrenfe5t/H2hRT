#pragma once


#include"LxQPoint3D.h"
namespace Point3DIStd {

	class Point3DI
	{
	public:
		int index;
		Point3DStd::Point3D p;
		Point3DI();
		Point3DI(int index, const Point3DStd::Point3D& p);
		~Point3DI();

	private:

	};


}