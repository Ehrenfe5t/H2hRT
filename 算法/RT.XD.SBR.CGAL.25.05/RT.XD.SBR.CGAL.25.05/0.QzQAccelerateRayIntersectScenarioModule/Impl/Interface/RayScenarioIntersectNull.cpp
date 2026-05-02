
#include"../Interface/RayScenarioIntersectNull.h"

#include<algorithm>

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
		Point3DStd::Point3D& res) {
		int index = -1;
		double dis = GlobalConstantStd::BoundingBoxLength;
		Point3DStd::Point3D curRes;
		double curDis;
		for (int loop = 0; loop < triangleAccelerateStructDatabase.triangle_real_size; ++loop) {
			if (1 != Geometry3DIntersectStd::Intersect_Ray3D_Triangle3D(
				ray,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1E2,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {
				continue;
			}

			if (curDis > GlobalConstantStd::Eps && curDis + GlobalConstantStd::Eps < dis) {
				dis = curDis;
				Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, curRes);
				index = loop;
			}
		}
		minDis = dis;

		return index;
	}


	Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult IntersectRayScenarioTriangle(
		const Ray3DStd::Ray3D& ray,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase) {
		Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult res;
		res.index = IntersectRayScenarioTrianglePlus(ray,res.distance, triangleAccelerateStructDatabase,res.point);
		return res;
	}

	int IntersectRayScenarioTriangleByIndexPlus(
		const Ray3DStd::Ray3D& ray,
		double& minDis,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		const std::vector<int>& triangles,
		Point3DStd::Point3D& res) {
		int index = -1;
		double dis = GlobalConstantStd::BoundingBoxLength;
		Point3DStd::Point3D curRes;
		double curDis;
		for (int loop_triangles = 0; loop_triangles < triangles.size(); ++loop_triangles) {
			int loop = triangles[loop_triangles];
			if (1 != Geometry3DIntersectStd::Intersect_Ray3D_Triangle3D(
				ray,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1E2,
				triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {
				continue;
			}

			if (curDis > GlobalConstantStd::Eps && curDis + GlobalConstantStd::Eps < dis) {
				dis = curDis;
				Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, curRes);
				index = loop;
			}
		}
		minDis = dis;

		return index;
	}

	Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult IntersectRayScenarioTriangleByIndex(
		const Ray3DStd::Ray3D& ray,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		const std::vector<int>& triangles) {
		Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult res;
		res.index = IntersectRayScenarioTriangleByIndexPlus(ray, res.distance, triangleAccelerateStructDatabase, triangles, res.point);
		return res;
	}

	int IntersectRayScenarioCornerPlus(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct,
		double& rayLength,
		Point3DStd::Point3D& res) {
		int indexMostNearCorner = -1;
		double disMostNearCorner = GlobalConstantStd::BoundingBoxLength;
		Point3DStd::Point3D curRes;
		Point3DStd::Point3D res1;
		Point3DStd::Point3D res2;

		for (int i = 0; i < scenarioAccelerateStruct->scenarioCorner3DIndex.size(); ++i) {

			Point3DStd::Point3D point_start = scenarioAccelerateStruct->scenarioPoint3D[scenarioAccelerateStruct->scenarioCorner3DIndex[i].P1Index];
			Point3DStd::Point3D point_end = scenarioAccelerateStruct->scenarioPoint3D[scenarioAccelerateStruct->scenarioCorner3DIndex[i].P2Index];


			Line3DStd::Line3D curLine = Geometry3DOperateStd::CreateLine3DByTwoPoint3D_unsafe(point_start, point_end);
			//先计算端点到线段的距离，不能太小
			double pToSeg = Geometry3DOperateStd::GetDistancePoint3DLine3D_unsafe(ray.o, curLine);
			if (pToSeg < radiusCorner) {
				continue;
			}
			double curSegSegDis = Geometry3DOperateStd::GetDistanceLine3DLine3D_plus_unsafe(
				ray.o,
				ray.vec,
				curLine.p,
				curLine.vec,
				res1,
				res2);
			if (!Geometry3DOperateStd::Location_Point3DonRay3D_SameLine3D_plus_safe(res1, ray.o, ray.vec)) {
				continue;
			}
			if (!Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(res2, point_start, point_end)) {
				continue;
			}

			if (curSegSegDis <= radiusCorner) {
				double rayoDistance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(ray.o, res2);
				if (disMostNearCorner > rayoDistance) {
					disMostNearCorner = rayoDistance;
					indexMostNearCorner = i;
					Geometry3DOperateStd::AssignmentPoint3DPoint3D(curRes, res2);
					rayLength = rayoDistance;
				}
			}

		}

		if (disMostNearCorner < GlobalConstantStd::BoundingBoxLength) {
			Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, curRes);
			return indexMostNearCorner;
		}
		return -1;
	}


	Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult IntersectRayScenarioCorner(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct) {
		Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult res;
		res.index = IntersectRayScenarioCornerPlus(radiusCorner, ray, scenarioAccelerateStruct, res.distance, res.point);
		return res;
	}


	int IntersectRayScenarioCornerByIndexPlus(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct,
		const std::vector<int>& cornerIndexes,
		double& rayLength,
		Point3DStd::Point3D& res) {
		int indexMostNearCorner = -1;
		double disMostNearCorner = GlobalConstantStd::BoundingBoxLength;
		Point3DStd::Point3D curRes;
		Point3DStd::Point3D res1;
		Point3DStd::Point3D res2;

		for (int loop_cornerIndexes = 0; loop_cornerIndexes < cornerIndexes.size(); ++loop_cornerIndexes) {
			int i = cornerIndexes[loop_cornerIndexes];
			Point3DStd::Point3D point_start = scenarioAccelerateStruct->scenarioPoint3D[scenarioAccelerateStruct->scenarioCorner3DIndex[i].P1Index];
			Point3DStd::Point3D point_end = scenarioAccelerateStruct->scenarioPoint3D[scenarioAccelerateStruct->scenarioCorner3DIndex[i].P2Index];


			Line3DStd::Line3D curLine = Geometry3DOperateStd::CreateLine3DByTwoPoint3D_unsafe(point_start, point_end);
			//先计算端点到线段的距离，不能太小
			double pToSeg = Geometry3DOperateStd::GetDistancePoint3DLine3D_unsafe(ray.o, curLine);
			if (pToSeg < radiusCorner) {
				continue;
			}
			double curSegSegDis = Geometry3DOperateStd::GetDistanceLine3DLine3D_plus_unsafe(
				ray.o,
				ray.vec,
				curLine.p,
				curLine.vec,
				res1,
				res2);
			if (!Geometry3DOperateStd::Location_Point3DonRay3D_SameLine3D_plus_safe(res1, ray.o, ray.vec)) {
				continue;
			}
			if (!Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(res2, point_start, point_end)) {
				continue;
			}

			if (curSegSegDis <= radiusCorner) {
				double rayoDistance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(ray.o, res2);
				if (disMostNearCorner > rayoDistance) {
					disMostNearCorner = rayoDistance;
					indexMostNearCorner = i;
					Geometry3DOperateStd::AssignmentPoint3DPoint3D(curRes, res2);
					rayLength = rayoDistance;
				}
			}

		}

		if (disMostNearCorner < GlobalConstantStd::BoundingBoxLength) {
			Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, curRes);
			return indexMostNearCorner;
		}
		return -1;
	}

	Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult IntersectRayScenarioCornerByIndex(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct,
		const std::vector<int>& cornerIndexes) {
		Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult res;
		res.index = IntersectRayScenarioCornerByIndexPlus(radiusCorner, ray, scenarioAccelerateStruct, cornerIndexes, res.distance, res.point);
		return res;
	}

	Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult IntersectRayScenarioTriangleAndCorner(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioAccelerateStruct,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase) {

		Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult res;
		res.ray3DTriangle3DIntersectResult = IntersectRayScenarioTriangle(ray, triangleAccelerateStructDatabase);
		auto ray3DCorner3DIntersectResult = IntersectRayScenarioCorner(radiusCorner, ray, scenarioAccelerateStruct);

		if (res.ray3DTriangle3DIntersectResult.distance > ray3DCorner3DIntersectResult.distance - GlobalConstantStd::Eps - radiusCorner) {
			res.ray3DCorner3DIntersectResult = ray3DCorner3DIntersectResult;
		}

		return res;
	}

	/// <summary>
	/// 射线和场景的所有三角形碰撞并且返回碰撞的所有结果,没有排序
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="sbrParameter"></param>
	/// <returns></returns>
	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> IntersectRay3DScenarioTriangle3DArray(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase) {
		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> res;
		
		{
			for (int loop = 0; loop < triangleAccelerateStructDatabase->triangle_real_size; ++loop) {
				Point3DStd::Point3D curRes;
				double curDis = GlobalConstantStd::BoundingBoxLength;
				if (1 != Geometry3DIntersectStd::Intersect_Ray3D_Triangle3D(
					ray,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE1,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE2,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE1E2,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE2E1,
					curDis,
					curRes)) {
					continue;
				}

				if (curDis > GlobalConstantStd::Eps && curDis + GlobalConstantStd::Eps < GlobalConstantStd::BoundingBoxLength) {
					Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
					ray3DTriangle3DIntersectResult.distance = curDis;
					ray3DTriangle3DIntersectResult.index = loop;
					ray3DTriangle3DIntersectResult.point = curRes;
					res.emplace_back(ray3DTriangle3DIntersectResult);
				}
			}
		}

		return res;
	}


	/// <summary>
	/// 射线和场景的所有三角形碰撞并且返回碰撞的所有结果,没有排序
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="sbrParameter"></param>
	/// <param name="triangleIndexes"></param>
	/// <returns></returns>
	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> IntersectRay3DScenarioTriangle3DArrayIndex(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase,
		const std::vector<int>& triangleIndexes) {
		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> res;

		{
			for (int loop_triangleIndexes = 0; loop_triangleIndexes < triangleIndexes.size(); ++loop_triangleIndexes) {
				int loop = triangleIndexes[loop_triangleIndexes];
				Point3DStd::Point3D curRes;
				double curDis = GlobalConstantStd::BoundingBoxLength;
				if (1 != Geometry3DIntersectStd::Intersect_Ray3D_Triangle3D(
					ray,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE1,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE2,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE1E2,
					triangleAccelerateStructDatabase->triangleAccelerateStructs[loop].scenarioE2E1,
					curDis,
					curRes)) {
					continue;
				}

				if (curDis > GlobalConstantStd::Eps && curDis + GlobalConstantStd::Eps < GlobalConstantStd::BoundingBoxLength) {
					Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
					ray3DTriangle3DIntersectResult.distance = curDis;
					ray3DTriangle3DIntersectResult.index = loop;
					ray3DTriangle3DIntersectResult.point = curRes;
					res.emplace_back(ray3DTriangle3DIntersectResult);
				}
			}
		}

		return res;
	}



	/// <summary>
	/// 射线和场景的所有三角形碰撞并且返回碰撞的所有结果,排序了
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="sbrParameter"></param>
	/// <param name="triangleIndexes"></param>
	/// <returns></returns>
	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> IntersectRay3DScenarioTriangle3DArrayIndexAndSort(
		const Ray3DStd::Ray3D& ray,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase,
		const std::vector<int>& triangleIndexes) {
		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> res;

		{
			for (int loop_triangleIndexes = 0; loop_triangleIndexes < triangleIndexes.size(); ++loop_triangleIndexes) {
				int loop = triangleIndexes[loop_triangleIndexes];
				Point3DStd::Point3D curRes;
				double curDis = GlobalConstantStd::BoundingBoxLength;
				if (1 != Geometry3DIntersectStd::Intersect_Ray3D_Triangle3D(
					ray,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE1E2,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioE2E1,
					curDis,
					curRes)) {
					continue;
				}

				if (curDis > GlobalConstantStd::Eps && curDis + GlobalConstantStd::Eps < GlobalConstantStd::BoundingBoxLength) {
					Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
					ray3DTriangle3DIntersectResult.distance = curDis;
					ray3DTriangle3DIntersectResult.index = loop;
					ray3DTriangle3DIntersectResult.point = curRes;
					res.emplace_back(ray3DTriangle3DIntersectResult);
				}
			}
		}


		std::sort(res.begin(), res.end(), [](const Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult& a,
			const Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult& b) {
				if (abs(a.distance - b.distance) < GlobalConstantStd::MoreEps) {
					if (a.index < b.index) {
						return true;
					}
				}
				else if (a.distance < b.distance) {
					return true;
				}
				return false;
			});
		return res;
	}

}