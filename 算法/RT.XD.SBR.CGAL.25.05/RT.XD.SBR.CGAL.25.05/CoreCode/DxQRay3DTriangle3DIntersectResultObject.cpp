
#include"DxQRay3DTriangle3DIntersectResultObject.h"
#include"QzQGlobalConstant.h"

namespace Ray3DTriangle3DIntersectResultObjectStd {

	Ray3DTriangle3DIntersectResultObject::Ray3DTriangle3DIntersectResultObject() {
		this->intersect = false;
		this->ray3DIntersectTriangle3DIndex = -1;
		this->ray3DIntersectTriangle3DDistance = GlobalConstantStd::BoundingBoxLength;

	}
	Ray3DTriangle3DIntersectResultObject::Ray3DTriangle3DIntersectResultObject(
		int ray3DIntersectTriangle3DIndex,
		double ray3DIntersectTriangle3DDistance,
		const Point3DStd::Point3D& ray3DIntersectTriangle3DPoint3D) {
		this->intersect = true;
		this->ray3DIntersectTriangle3DIndex = ray3DIntersectTriangle3DIndex;
		this->ray3DIntersectTriangle3DDistance = ray3DIntersectTriangle3DDistance;
		this->ray3DIntersectTriangle3DPoint3D = ray3DIntersectTriangle3DPoint3D;
	}
	Ray3DTriangle3DIntersectResultObject::~Ray3DTriangle3DIntersectResultObject() {

	}

}