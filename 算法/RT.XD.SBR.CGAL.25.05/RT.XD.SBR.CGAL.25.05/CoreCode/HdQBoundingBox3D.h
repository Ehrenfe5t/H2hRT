#pragma once



#include"LxQPoint3D.h"

namespace BoundingBox3DStd {

	class BoundingBox3D
	{
	public:
		/// <summary>
		/// 射线端点
		/// </summary>
		Point3DStd::Point3D min;
		/// <summary>
		/// 射线方向向量
		/// </summary>
		Point3DStd::Point3D max;


		BoundingBox3D();
		BoundingBox3D(const BoundingBox3D& obj);
		BoundingBox3D(const Point3DStd::Point3D& min, const Point3DStd::Point3D& max);
		~BoundingBox3D();

		bool IsXInBox(double x);
		bool IsYInBox(double y);
		bool IsZInBox(double z);
		bool IsPoint3DInBox(const Point3DStd::Point3D& obj);
		bool IsColliding(const BoundingBox3DStd::BoundingBox3D& box);
	private:

	};

}

