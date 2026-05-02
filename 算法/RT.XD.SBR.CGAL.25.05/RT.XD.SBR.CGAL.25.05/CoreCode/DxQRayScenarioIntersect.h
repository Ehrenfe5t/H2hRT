#pragma once

#include"HdQConfig.h"
#include"DxQRayScenarioIntersectType.h"
#include"DxQRay3DTriangle3DIntersectResult.h"
#include"DxQRay3D.h"
#include"DxQRay3DScenario3DIntersectResult.h"
#include"DxQScenarioObject.h"
#include"DxQSpaceSubregionParameterConfig.h"
#include"DxQTriangleAccelerateStructDatabase.h"


namespace RayScenarioIntersectStd {
	
	/// <summary>
	/// 必须初始化，目前的所有代码可以更新，null,Pixel3D,Pixel3D_sdf,BvhBall,共计4种模式
	/// </summary>
	/// <param name="type"></param>
	/// <param name="corner"></param>
	/// <param name="stepLength"></param>
	/// <param name="scenarioObject"></param>
	/// <param name="ray3DIntersectScenarioAccelerateStruct"></param>
	/// <returns></returns>
	INTERFACE_API bool SetPixelType(const RayScenarioIntersectTypeStd::RayScenarioIntersectType& type,
		bool corner,
		const SpaceSubregionParameterConfigStd::SpaceSubregionParameterConfig& spaceSubregionParameterConfig,
		const ScenarioObjectStd::ScenarioObject& scenarioObject,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase);


	/// <summary>
	/// 计算射线和场景碰撞的所有结果，即射线和三角形碰撞的所有结果按照碰撞距离排序
	/// 如果有相同位置的碰撞，则需要进行删除；
	/// 1、两个面元的边缝，实际上这两个面元是一个平面；删除排序在后面的那一个碰撞结果
	/// 2、两个面元的边缝，实际上这两个面元在这个边缝构成了一个棱边；分为两种情况，可能删除1个；也可能都删除
	/// 3、其他情况(如面元重叠)暂时不处理
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="ray3DIntersectScenarioAccelerateStruct"></param>
	/// <returns></returns>
	INTERFACE_API std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> RayTriangle3DIntersectAndSort(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase);


	INTERFACE_API Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult RayScenarioTriangle3DIntersect(
		const Ray3DStd::Ray3D& ray,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase);


	INTERFACE_API Ray3DScenario3DIntersectResultStd::Ray3DScenario3DIntersectResult RayScenarioTriangle3DAndCorner3DIntersect(
		double radiusCorner,
		const Ray3DStd::Ray3D& ray,
		ScenarioObjectStd::ScenarioObject* scenarioObject,
		TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase* triangleAccelerateStructDatabase);

	INTERFACE_API std::vector<Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult> Ray3DsPlane3DsIntersect(
		const Ray3DStd::Ray3D& ray1,
		const Ray3DStd::Ray3D& ray2,
		const Ray3DStd::Ray3D& ray3,
		const TriangleAccelerateStructDatabaseStd::TriangleAccelerateStructDatabase& triangleAccelerateStructDatabase);

}
