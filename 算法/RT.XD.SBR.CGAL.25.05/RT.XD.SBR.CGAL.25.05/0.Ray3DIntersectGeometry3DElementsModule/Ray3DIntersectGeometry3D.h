#pragma once

#include"0.Ray3DIntersectGeometry3DElementsModule.Input.h"
#include"Ray3DIntersectGeometry3DElementsResults.h"

namespace Ray3DIntersectGeometry3DStd {

	void Initialize(int type, const Scenario3D& scenario, bool switchCorner, double cornerRadius);

	void CalculateRay3DIntersectTriangle3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectCorner3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results);


	void CalculateRay3DIntersectTriangle3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementsResults& results);


	void CalculateRay3DIntersectTriangle3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result);


}

/// <summary>
/// 当前没有使用的函数
/// </summary>
namespace Ray3DIntersectGeometry3DStd {

	void InitializeDefault(const Scenario3D& scenario, bool switchCorner, double cornerRadius);

	void CalculateRay3DIntersectCorner3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results);

	/// <summary>
	/// 射线和整个场景的相交检测，暴力方法
	/// </summary>
	/// <param name="ray_origin">射线的起点</param>
	/// <param name="ray_direction">射线的方向，这里是归一化的</param>
	/// <param name="cornerRadius">边的半径</param>
	/// <param name="scenario">环境的几何信息</param>
	/// <param name="results">计算结果</param>
	void CalculateRay3DIntersectScenario3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results);


	void CalculateRay3DIntersectScenario3DSort(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementsResults& results);


	void CalculateRay3DIntersectScenario3DFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result);
}

