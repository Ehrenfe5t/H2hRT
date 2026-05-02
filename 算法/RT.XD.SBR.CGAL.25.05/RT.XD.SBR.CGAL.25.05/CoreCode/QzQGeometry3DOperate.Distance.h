#pragma once




#include"HdQBall3D.h"
#include"HdQBaseCoordinateSystem.h"
#include"HdQBoundingBox3D.h"
#include"HdQConfig.h"
#include"LxQLine3D.h"
#include"LxQLineSegment3D.h"
#include"LxQPlane3D.h"
#include"LxQPoint2I.h"
#include"LxQPoint3DI.h"
#include"LxQPoint3F.h"
#include"LxQPoint3I.h"
#include"LxQPoint2D.h"
#include"LxQPoint3D.h"
#include"DxQRay3D.h"
#include"DxQTriangle3D.h"
#include"DxQTriangleI.h"



namespace Geometry3DOperateStd {

	INTERFACE_API double GetDistancePoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);

	INTERFACE_API double GetDistancePoint3DLine3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO,
		const Point3DStd::Point3D& lineVec);

	INTERFACE_API bool GetDistancePoint3DLine3D_plus_safe(const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec, double& distance);

	INTERFACE_API double GetDistancePoint3DLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line);

	INTERFACE_API bool GetDistancePoint3DLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line,
		double& distance);


	INTERFACE_API double GetDistancePoint3DRay3D_unsafe(const Point3DStd::Point3D& p, const Ray3DStd::Ray3D& ray);

	INTERFACE_API bool GetDistancePoint3DRay3D_safe(const Point3DStd::Point3D& p,
		const Ray3DStd::Ray3D& ray, double& distance);

	INTERFACE_API double GetDistanceLine3DLine3D_plus_unsafe(
		const Point3DStd::Point3D& o1,
		const Point3DStd::Point3D& d1,
		const Point3DStd::Point3D& o2,
		const Point3DStd::Point3D& d2,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2);

	INTERFACE_API double GetDistanceLine3DLine3D_unsafe(
		const Line3DStd::Line3D& line1,
		const Line3DStd::Line3D& line2,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2);

	INTERFACE_API double GetDistancePoint3DPlane3D_plus_unsafe(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& planep,
		const Point3DStd::Point3D& planen);

	INTERFACE_API bool GetDistancePoint3DPlane3D_plus_safe(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& planep,
		const Point3DStd::Point3D& planen, double& distance);

	INTERFACE_API double GetDistancePoint3DPlane3D_unsafe(const Point3DStd::Point3D& p,
		const Plane3DStd::Plane3D& plane);

	INTERFACE_API bool GetDistancePoint3DPlane3D_safe(const Point3DStd::Point3D& p,
		const Plane3DStd::Plane3D& plane, double& distance);

	INTERFACE_API double GetDistancePoint3DLineSegment3D_plus(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& segStart,
		const Point3DStd::Point3D& segEnd);
	INTERFACE_API double GetDistancePoint3DLineSegment3D(const Point3DStd::Point3D& p, const LineSegment3DStd::LineSegment3D& seg);


}