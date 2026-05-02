#pragma once

#include"../Input.h"


namespace RayScenarioIntersectNullStd {


	/// <summary>
	/// 和全场景的三角形交互，没有使用空间加速
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="minDis"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	int IntersectRayScenarioTrianglePlus(
		const Ray3DStd::Ray3D& ray,
		double& minDis,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		Point3DStd::Point3D& res);

	Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult IntersectRayScenarioTriangle(
		const Ray3DStd::Ray3D& ray,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase);

	int IntersectRayScenarioTriangleByIndexPlus(
		const Ray3DStd::Ray3D& ray,
		double& minDis,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		const std::vector<int>& triangles,
		Point3DStd::Point3D& res);

	Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult IntersectRayScenarioTriangleByIndex(
		const Ray3DStd::Ray3D& ray,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		const std::vector<int>& triangles);

	int IntersectRayScenarioCornerPlus(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject,
		double& rayLength,
		Point3DStd::Point3D& res);


	Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult IntersectRayScenarioCorner(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject );


	int IntersectRayScenarioCornerByIndexPlus(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject,
		const std::vector<int>& cornerIndexes,
		double& rayLength,
		Point3DStd::Point3D& res);

	Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult IntersectRayScenarioCornerByIndex(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject,
		const std::vector<int>& cornerIndexes);

	Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult IntersectRayScenarioTriangleAndCorner(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase);


	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> IntersectRay3DScenarioTriangle3DArray(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase);


	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> IntersectRay3DScenarioTriangle3DArrayIndex(
		const Ray3DStd::Ray3D& ray, 
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase,
		const std::vector<int>& triangleIndexes);

}
