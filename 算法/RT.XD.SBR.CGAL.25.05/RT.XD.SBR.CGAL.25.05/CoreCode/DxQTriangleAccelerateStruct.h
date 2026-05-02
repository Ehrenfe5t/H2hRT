#pragma once

#include"HdQBoundingBox3D.h"

namespace TriangleAccelerateStructStd {
	
	class TriangleAccelerateStruct
	{
	public:

		int upTypeNumber;
		int downTypeNumber;
		float roughness;

		Point3DStd::Point3D scenarioTriangleP1;
		Point3DStd::Point3D scenarioTriangleP2;
		Point3DStd::Point3D scenarioTriangleP3;
		Point3DStd::Point3D scenarioTriangleN;
		/// <summary>
		/// 场景碰撞时计算的量
		/// </summary>
		Point3DStd::Point3D scenarioE1;

		/// <summary>
		/// 场景碰撞时计算的量
		/// </summary>
		Point3DStd::Point3D scenarioE2;
		Point3DStd::Point3D scenarioE1E2;
		Point3DStd::Point3D scenarioE2E1;

		BoundingBox3DStd::BoundingBox3D triangleBox;
		TriangleAccelerateStruct();
		~TriangleAccelerateStruct();

	private:

	};



}