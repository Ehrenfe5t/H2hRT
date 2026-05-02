#pragma once


#include"LxQPoint3D.h"

namespace BaseCoordinateSystemStd {
	/// <summary>
	/// »ù×ø±êÏµ
	/// </summary>
	class BaseCoordinateSystem
	{
	public:
		Point3DStd::Point3D o;
		Point3DStd::Point3D x;
		Point3DStd::Point3D y;
		Point3DStd::Point3D z;

		BaseCoordinateSystem();
		BaseCoordinateSystem(const Point3DStd::Point3D& o, const Point3DStd::Point3D& x, const Point3DStd::Point3D& y, const Point3DStd::Point3D& z);
		~BaseCoordinateSystem();

	private:

	};

}