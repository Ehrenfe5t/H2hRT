
#include"DxQRayScenarioIntersectResult.h"

#include"QzQGlobalConstant.h"
namespace RayScenarioIntersectResultStd {

	RayScenarioIntersectResult::RayScenarioIntersectResult()
	{
		this->intersectType = -1;
		this->triangleDistance = GlobalConstantStd::BoundingBoxLength;
		this->cornerDistance = GlobalConstantStd::BoundingBoxLength;
		this->triangleIndex = 0;
		this->cornerIndex = 0;


	}


	RayScenarioIntersectResult::RayScenarioIntersectResult(int intersectType,double triangleDistance, int triangleIndex, const Point3DStd::Point3D& triangleIntersectResult)
	{
		if (intersectType == 1) {
			this->intersectType = 1;
			this->cornerDistance = GlobalConstantStd::BoundingBoxLength;
			this->cornerIndex = 0;
			this->triangleDistance = triangleDistance;
			this->triangleIndex = triangleIndex;
			this->triangleIntersectResult = triangleIntersectResult;
		}
		else if (intersectType == 2) {
			this->intersectType = 2;
			this->triangleDistance = GlobalConstantStd::BoundingBoxLength;
			this->triangleIndex = 0;
			this->cornerDistance = triangleDistance;
			this->cornerIndex = triangleIndex;
			this->cornerIntersectResult = triangleIntersectResult;
		}
		else {
			this->intersectType = -1;
			this->triangleDistance = GlobalConstantStd::BoundingBoxLength;
			this->cornerDistance = GlobalConstantStd::BoundingBoxLength;
			this->triangleIndex = 0;
			this->cornerIndex = 0;
		}
	}



	RayScenarioIntersectResult::RayScenarioIntersectResult(double triangleDistance, int triangleIndex, const Point3DStd::Point3D& triangleIntersectResult,
		double cornerDistance, int cornerIndex, const Point3DStd::Point3D& cornerIntersectResult)
	{
		this->intersectType = 3;
		this->triangleDistance = triangleDistance;
		this->triangleIndex = triangleIndex;
		this->triangleIntersectResult = triangleIntersectResult;
		this->cornerDistance = cornerDistance;
		this->cornerIndex = cornerIndex;
		this->cornerIntersectResult = cornerIntersectResult;
	}

	RayScenarioIntersectResult::~RayScenarioIntersectResult()
	{
	}
}