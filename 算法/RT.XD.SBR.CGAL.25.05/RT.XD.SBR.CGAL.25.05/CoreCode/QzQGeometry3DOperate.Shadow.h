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

	INTERFACE_API Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(const Point3DStd::Point3D& obj, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec);
	INTERFACE_API bool CalculateShadowPoint3DOfPoint3DonLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec, Point3DStd::Point3D& res);
	INTERFACE_API Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line);
	INTERFACE_API bool CalculateShadowPoint3DOfPoint3DonLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line, Point3DStd::Point3D& res);
	INTERFACE_API bool CalculateShadowPoint3DOfPoint3DonRay3D_safe(const Point3DStd::Point3D& a, const Ray3DStd::Ray3D& ray, Point3DStd::Point3D& res);
	INTERFACE_API Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(const Point3DStd::Point3D& a, const  Point3DStd::Point3D& p, const Point3DStd::Point3D& n);
	INTERFACE_API bool CalculateShadowPoint3DOfPoint3DonPlane3D_plus_safe(const Point3DStd::Point3D& a, const  Point3DStd::Point3D& p, const Point3DStd::Point3D& n, Point3DStd::Point3D& res);
	INTERFACE_API Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonPlane3D_unsafe(const Point3DStd::Point3D& a, const  Plane3DStd::Plane3D& plane);
	INTERFACE_API bool CalculateShadowPoint3DOfPoint3DonPlane3D_safe(const Point3DStd::Point3D& a, const  Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res);

	/// <summary>
	///  计算点在直线段上的投影点，-1表示不在线段上，0表示在线段的端点上，1表示在线段内
	/// </summary>
	/// <param name="obj_location"></param>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="shadow_location"></param>
	/// <returns></returns>
	INTERFACE_API int CalculateShadowPoint3DOfPoint3DonLineSegment3D_plus_safe(
		const Point3DStd::Point3D& obj_location,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		Point3DStd::Point3D& shadow_location);
}