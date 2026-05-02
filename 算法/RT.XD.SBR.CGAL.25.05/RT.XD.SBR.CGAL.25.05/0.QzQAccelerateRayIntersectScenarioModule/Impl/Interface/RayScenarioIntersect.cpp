
#include"../Input.h"
#include"../Interface/RayScenarioIntersectNull.h"
#include<iostream>
#include<algorithm>

namespace RayScenarioIntersectStd {


	RayScenarioIntersectTypeStd::RayScenarioIntersectType Type;

	bool SetPixelType(const RayScenarioIntersectTypeStd::RayScenarioIntersectType& type,
		bool corner,
		const SpaceSubregionParameterConfigStd::SpaceSubregionParameterConfig& spaceSubregionParameterConfig,
		const ScenarioObjectStd::ScenarioObject& scenarioObject,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase) {
		Type = type;

		switch (Type)
		{
		case RayScenarioIntersectTypeStd::RayScenarioIntersectType::Null:
			return true;
		default:
			break;
		}
		return false;
	}


	/// <summary>
	/// 射线和场景的所有三角形碰撞并且返回碰撞的所有结果
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="scenarioParameter"></param>
	/// <returns></returns>
	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> RayTriangle3DIntersectAndSort(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase) {

		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> res;

		switch (Type)
		{
		case RayScenarioIntersectTypeStd::RayScenarioIntersectType::Null:
			res = RayScenarioIntersectNullStd::IntersectRay3DScenarioTriangle3DArray(ray, triangleAccelerateStructDatabase);
			break;
		default:
			break;
		}

		//对结果应该排序并可能进行删除

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


		//如果有相同位置的碰撞，则需要进行删除；
		// 1、两个面元的边缝，实际上这两个面元是一个平面；删除其中排序在后面的那一个碰撞结果
		// 2、两个面元的边缝，实际上这两个面元在这个边缝构成了一个棱边；分为两种情况，可能删除1个；也可能都删除
		// 3、其他情况(如面元重叠)暂时不处理

		

		for (int i = (int)res.size() - 1; i > 0; --i) {

			int pre = i - 1;
			if (Geometry3DOperateStd::GetDistancePoint3DPoint3D(res[i].point, res[pre].point)<1e-4) {
				auto tringle1N = triangleAccelerateStructDatabase->triangleAccelerateStructs[res[i].index].scenarioTriangleN;
				auto tringle2N = triangleAccelerateStructDatabase->triangleAccelerateStructs[res[pre].index].scenarioTriangleN;

				//处理情况1
				if (Geometry3DOperateStd::Equals_Point3D(tringle1N, tringle2N)) {
					res.erase(res.begin() + i);
					continue;
				}

				//处理情况2

				auto dot1 = Geometry3DOperateStd::DotPoint3DPoint3D(ray.vec, tringle1N);
				auto dot2 = Geometry3DOperateStd::DotPoint3DPoint3D(ray.vec, tringle2N);
				if (dot1>0 && dot2>0) {
					res.erase(res.begin() + i);
					continue;
				}
				else if (dot1 < 0 && dot2 < 0) {
					res.erase(res.begin() + i);
					continue;
				}
				else {
					res.erase(res.begin() + i);
					i--;
					res.erase(res.begin() + i);
					continue;
				}
			}

		}

		return res;


	}



	Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult RayScenarioTriangle3DIntersect(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase) {

		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> results = RayTriangle3DIntersectAndSort(ray, triangleAccelerateStructDatabase);

		Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult res;
		if (results.size() > 0) {
			res.distance = results[0].distance;
			res.index = results[0].index;
			res.point = results[0].point;
		}
		return res;
	}


	Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult RayScenarioTriangle3DAndCorner3DIntersect(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase) {

		Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult res;

		{

			std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> triangle3D_results = RayTriangle3DIntersectAndSort(ray, triangleAccelerateStructDatabase);

			if (triangle3D_results.size() > 0) {
				res.ray3DTriangle3DIntersectResult.distance = triangle3D_results[0].distance;
				res.ray3DTriangle3DIntersectResult.index = triangle3D_results[0].index;
				res.ray3DTriangle3DIntersectResult.point = triangle3D_results[0].point;
			}
			Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult ray3DCorner3DIntersectResult;
			switch (Type)
			{
			case RayScenarioIntersectTypeStd::RayScenarioIntersectType::Null:
				ray3DCorner3DIntersectResult = RayScenarioIntersectNullStd::IntersectRayScenarioCorner(
					radiusCorner, ray, scenarioObject);
				break;

			default:
				break;
			}


			if (res.ray3DTriangle3DIntersectResult.distance > ray3DCorner3DIntersectResult.distance - GlobalConstantStd::Eps - radiusCorner) {
				res.ray3DCorner3DIntersectResult = ray3DCorner3DIntersectResult;
			}
		}


		return res;
	}


	bool is_colliding(
		const BoundingBox3DStd::BoundingBox3D& box1, const BoundingBox3DStd::BoundingBox3D& box2) {
		return (box1.min.x <= box2.max.x and box1.max.x >= box2.min.x)
			&& (box1.min.y <= box2.max.y and box1.max.y >= box2.min.y)
			&& (box1.min.z <= box2.max.z and box1.max.z >= box2.min.z);
	}

	/// <summary>
	/// 计算射线和平面的交点
	/// </summary>
	/// <param name="ray1"></param>
	/// <param name="ray2"></param>
	/// <param name="ray3"></param>
	/// <param name="scenarioParameter"></param>
	/// <returns></returns>
	std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> Ray3DsPlane3DsIntersect(
		const Ray3DStd::Ray3D& ray1,
		const Ray3DStd::Ray3D& ray2,
		const Ray3DStd::Ray3D& ray3,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase) {

		std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> res;
		auto rayOcenter = Geometry3DOperateStd::Center_Point3D_Point3D_Point3D(ray1.o, ray2.o, ray3.o);
		{
			for (int loop = 0; loop < triangleAccelerateStructDatabase.triangle_real_size; ++loop) {
				Point3DStd::Point3D curRes1;
				Point3DStd::Point3D curRes2;
				Point3DStd::Point3D curRes3;
				int state_sum = 0;
				bool state1 = Geometry3DIntersectStd::Intersect_Ray3D_Plane3D_plus_plus(
					ray1.o,
					ray1.vec,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleN,
					curRes1);
				bool state2 = Geometry3DIntersectStd::Intersect_Ray3D_Plane3D_plus_plus(
					ray2.o,
					ray2.vec,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleN,
					curRes2);
				bool state3 = Geometry3DIntersectStd::Intersect_Ray3D_Plane3D_plus_plus(
					ray3.o,
					ray3.vec,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleP1,
					triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].scenarioTriangleN,
					curRes3);

				if (state1) {
					state_sum++;
				}
				if (state2) {
					state_sum++;
				}
				if (state3) {
					state_sum++;
				}
				if (state_sum > 2) {

					BoundingBox3DStd::BoundingBox3D curBoundingBox3D = Geometry3DOperateStd::CreateBoundingBox3DBy3Point3D(curRes1, curRes2, curRes3);

					if (is_colliding(curBoundingBox3D, triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].triangleBox)) {

						Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
						ray3DTriangle3DIntersectResult.index = loop;
						ray3DTriangle3DIntersectResult.point = Geometry3DOperateStd::Center_Point3D_Point3D_Point3D(curRes1, curRes2, curRes3);
						ray3DTriangle3DIntersectResult.distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(ray3DTriangle3DIntersectResult.point, rayOcenter);
						res.emplace_back(ray3DTriangle3DIntersectResult);

					}

				}
				else {

					if (state_sum < 1) {
						continue;
					}
					else {
						if (!state1) {
							curRes1 = Geometry3DOperateStd::AddPoint3DPoint3D(ray1.o, Geometry3DOperateStd::MulDoublePoint3D(GlobalConstantStd::BoundingBoxLength, ray1.vec));
						}
						if (!state2) {
							curRes2 = Geometry3DOperateStd::AddPoint3DPoint3D(ray2.o, Geometry3DOperateStd::MulDoublePoint3D(GlobalConstantStd::BoundingBoxLength, ray2.vec));
						}
						if (!state3) {
							curRes3 = Geometry3DOperateStd::AddPoint3DPoint3D(ray3.o, Geometry3DOperateStd::MulDoublePoint3D(GlobalConstantStd::BoundingBoxLength, ray3.vec));
						}

						BoundingBox3DStd::BoundingBox3D curBoundingBox3D = Geometry3DOperateStd::CreateBoundingBox3DBy3Point3D(curRes1, curRes2, curRes3);

						if (is_colliding(curBoundingBox3D, triangleAccelerateStructDatabase.triangleAccelerateStructs[loop].triangleBox)) {

							Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
							ray3DTriangle3DIntersectResult.index = loop;
							ray3DTriangle3DIntersectResult.point = Geometry3DOperateStd::Center_Point3D_Point3D_Point3D(curRes1, curRes2, curRes3);
							ray3DTriangle3DIntersectResult.distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(ray3DTriangle3DIntersectResult.point, rayOcenter);
							res.emplace_back(ray3DTriangle3DIntersectResult);
						}
					}
				}

			}
		}

		return res;
	}


}