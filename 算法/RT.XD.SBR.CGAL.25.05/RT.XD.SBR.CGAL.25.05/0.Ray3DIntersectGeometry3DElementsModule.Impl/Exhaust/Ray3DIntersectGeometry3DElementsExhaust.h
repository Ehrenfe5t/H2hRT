#pragma once

#include "../0.Ray3DIntersectGeometry3DElementsModule.Impl.Input.h"

namespace Ray3DIntersectGeometry3DElementsExhaustStd {

	void InitializeExhaust(const Scenario3D& scenario, bool switchCorner);

	/// <summary>
	/// 射线和整个场景的相交检测，暴力方法
	/// </summary>
	/// <param name="ray_origin">射线的起点</param>
	/// <param name="ray_direction">射线的方向，这里是归一化的</param>
	/// <param name="cornerRadius">边的半径</param>
	/// <param name="scenario">环境的几何信息</param>
	/// <param name="results">计算结果</param>
	void CalculateRay3DIntersectScenario3DExhaust(
		const Point3D& ray_origin, 
		const Point3D& ray_direction,
		double cornerRadius, 
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectTriangle3DExhaust(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectTriangle3DExhaustFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DExhaust(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectCorner3DExhaustFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result);

	
}