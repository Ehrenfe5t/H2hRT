#pragma once


#include "0.SolveOneTimeDiffractionPathByEquationModule.Impl.Input.h"

namespace SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd {


	struct SotdpbeRay3DIntersectGeometry3DElementResult
	{
		/// <summary>
		/// 碰撞类型，0表示和三角形碰撞了，1表示和边碰撞了
		/// </summary>
		int type;
		/// <summary>
		/// 碰撞的元素编号
		/// </summary>
		int elementIndex;
		double distance;
		Point3D location;

	};

	void InitializeAabbBvhTree(const Scenario3D& scenario);

	void CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		SotdpbeRay3DIntersectGeometry3DElementResult& result);

}