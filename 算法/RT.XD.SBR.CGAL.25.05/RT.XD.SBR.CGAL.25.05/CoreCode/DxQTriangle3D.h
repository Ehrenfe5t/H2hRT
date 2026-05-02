#pragma once


#include"LxQPoint3D.h"

namespace Triangle3DStd {
	/// <summary>
	/// 三维几何三角形
	/// </summary>
	class Triangle3D
	{
	public:
		/// <summary>
		/// 三角形的p1点
		/// </summary>
		Point3DStd::Point3D p1;
		/// <summary>
		/// 三角形的p2点
		/// </summary>
		Point3DStd::Point3D p2;
		/// <summary>
		/// 三角形的p3点
		/// </summary>
		Point3DStd::Point3D p3;
		Triangle3D();
		Triangle3D(const Triangle3DStd::Triangle3D& obj);
		Triangle3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3);
		~Triangle3D();

	private:

	};



}


