#pragma once

#include"LxQPoint3D.h"

namespace Ray3DTriangle3DIntersectResultStd {

	class Ray3DTriangle3DIntersectResult
	{
	public:

		/// <summary>
		/// 碰撞面元的编号
		/// </summary>
		int index;

		/// <summary>
		/// 射线碰撞时射线走的长度
		/// </summary>
		double distance;

		/// <summary>
		/// 碰撞点坐标
		/// </summary>
		Point3DStd::Point3D point;
		Ray3DTriangle3DIntersectResult();
		~Ray3DTriangle3DIntersectResult();

	private:

	};


}