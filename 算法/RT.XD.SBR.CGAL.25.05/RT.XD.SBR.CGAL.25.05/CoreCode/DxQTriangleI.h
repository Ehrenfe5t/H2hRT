#pragma once

#include"LxQPoint3D.h"

namespace TriangleIStd {


	class TriangleI
	{
	public:
		int index;
		Point3DStd::Point3D p1;
		Point3DStd::Point3D p2;
		Point3DStd::Point3D p3;
		Point3DStd::Point3D n;

		TriangleI();
		TriangleI(int index, const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3, const Point3DStd::Point3D& n);
		~TriangleI();

	private:

	};


	Point3DStd::Point3D GetCenter(const TriangleIStd::TriangleI& obj);


}