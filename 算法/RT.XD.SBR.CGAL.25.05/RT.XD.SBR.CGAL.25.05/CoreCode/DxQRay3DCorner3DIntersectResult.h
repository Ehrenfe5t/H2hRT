#pragma once

#include"LxQPoint3D.h"

namespace Ray3DCorner3DIntersectResultStd {

	class Ray3DCorner3DIntersectResult
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
		Ray3DCorner3DIntersectResult();
		~Ray3DCorner3DIntersectResult();

	private:

	};


}