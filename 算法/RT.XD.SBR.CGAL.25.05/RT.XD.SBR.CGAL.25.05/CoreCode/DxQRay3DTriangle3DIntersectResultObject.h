#pragma once

#include"LxQPoint3D.h"

namespace Ray3DTriangle3DIntersectResultObjectStd {

	/// <summary>
	/// 射线和三角形碰撞结果，
	/// </summary>
	class Ray3DTriangle3DIntersectResultObject {
	public:
		/// <summary>
		/// 是否碰撞
		/// </summary>
		bool intersect;
		/// <summary>
		/// 碰撞的三角形的编号
		/// </summary>
		int ray3DIntersectTriangle3DIndex;
		/// <summary>
		/// 
		/// </summary>
		double ray3DIntersectTriangle3DDistance;
		/// <summary>
		/// 碰撞点
		/// </summary>
		Point3DStd::Point3D ray3DIntersectTriangle3DPoint3D;

		Ray3DTriangle3DIntersectResultObject();
		Ray3DTriangle3DIntersectResultObject(
			int ray3DIntersectTriangle3DIndex, 
			double ray3DIntersectTriangle3DDistance,
			const Point3DStd::Point3D& ray3DIntersectTriangle3DPoint3D);
		~Ray3DTriangle3DIntersectResultObject();
	};

}