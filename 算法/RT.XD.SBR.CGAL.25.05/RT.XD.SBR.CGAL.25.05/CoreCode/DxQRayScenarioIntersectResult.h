#pragma once

#include"LxQPoint3D.h"

namespace RayScenarioIntersectResultStd {
	class RayScenarioIntersectResult
	{
	public:
		/// <summary>
		/// 碰撞类型、-1表示没有碰撞,1表示面元、2表示棱(并且没有和面碰撞)、3表示先和边碰撞，再和楞碰撞
		/// </summary>
		int intersectType;

		double triangleDistance;
		double cornerDistance;
		/// <summary>
		/// 碰撞的面元编号，
		/// </summary>
		int triangleIndex;
		/// <summary>
		/// 碰撞的边编号，
		/// </summary>
		int cornerIndex;

		Point3DStd::Point3D triangleIntersectResult;

		Point3DStd::Point3D cornerIntersectResult;


		RayScenarioIntersectResult();
		
		RayScenarioIntersectResult(int intersectType,double triangleDistance, int triangleIndex, const Point3DStd::Point3D& triangleIntersectResult);
		

		RayScenarioIntersectResult(double triangleDistance, int triangleIndex, const Point3DStd::Point3D& triangleIntersectResult,
			double cornerDistance, int cornerIndex, const Point3DStd::Point3D& cornerIntersectResult);

		~RayScenarioIntersectResult();
		

	private:

	};




}