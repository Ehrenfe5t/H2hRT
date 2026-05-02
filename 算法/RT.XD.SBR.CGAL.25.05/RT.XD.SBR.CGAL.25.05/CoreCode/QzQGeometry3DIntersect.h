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




namespace Geometry3DIntersectStd {

	//3D点 

	INTERFACE_API bool Intersect_Point3D_Point3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);

	INTERFACE_API bool Intersect_Point3D_Line3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO,
		const Point3DStd::Point3D& lineVec);

	INTERFACE_API bool Intersect_Point3D_Ray3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec);
	//3D直线


	INTERFACE_API bool Intersect_Line3D_Line3D_plus_plus_base(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		double eps,
		double& distance,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2);


	INTERFACE_API bool Intersect_Line3D_Line3D_plus_plus_2(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		double eps,
		Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_Line3D_plus_plus_1(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_LineSegment3D_plus_plus_2(
		const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		double eps,
		Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_LineSegment3D_plus_plus_1(
		const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		Point3DStd::Point3D& res);

	/// <summary>
	///  -1射线和线段无交点；0表示交点在射线端点；1表示有一个在射线上的交点，并且不在射线端点上
	/// </summary>
	/// <param name="rayO"></param>
	/// <param name="rayVec"></param>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="eps"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	INTERFACE_API int Intersect_Ray3D_LineSegment3D_plus_plus_2(
		const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		double eps,
		double& distance,
		Point3DStd::Point3D& res);

	/// <summary>
	/// -1射线和线段无交点；0表示交点在射线端点；1表示有一个在射线上的交点，并且不在射线端点上
	/// </summary>
	/// <param name="rayO"></param>
	/// <param name="rayVec"></param>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	INTERFACE_API int Intersect_Ray3D_LineSegment3D_plus_plus_1(
		const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_Plane3D_plus_plus(const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_Plane3D(const Line3DStd::Line3D& line, const Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_Line3D_Plane3D_plus_plus_d(const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res, double& distance);

	//3D线段

	/// <summary>
	/// 只有有且仅有一个交点时 返回true
	/// </summary>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="planeO"></param>
	/// <param name="planeN"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	INTERFACE_API bool Intersect_LineSegment3D_Plane3D_plus_plus(const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res);

	INTERFACE_API bool Intersect_LineSegment3D_LineSegment3D_plus(const Point3DStd::Point3D& seg1Start, const Point3DStd::Point3D& seg1End,
		const Point3DStd::Point3D& seg2Start, const Point3DStd::Point3D& seg2End);

	//3D射线

	INTERFACE_API bool Intersect_Ray3D_Plane3D_plus_plus(const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res);



	INTERFACE_API bool Intersect_Ray3D_Plane3D_plus_plus_d(const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res, double& distance);

	INTERFACE_API bool Intersect_Ray3D_Plane3D(const Ray3DStd::Ray3D& ray, const Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res);

	INTERFACE_API int Intersect_Ray3D_Triangle3D_plus2(
		const Point3DStd::Point3D& rayO,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& trianglep1,
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2,
		double& distance,
		Point3DStd::Point3D& res);

	INTERFACE_API int Intersect_Ray3D_Triangle3D_plus(
		const Point3DStd::Point3D& O,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& V1,
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2,
		const Point3DStd::Point3D& E1E2,
		const Point3DStd::Point3D& E2E1,
		double& distance,
		Point3DStd::Point3D& res);

	INTERFACE_API int Intersect_Ray3D_Triangle3D(
		const Ray3DStd::Ray3D& ray,
		const Point3DStd::Point3D& V1,
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2,
		const Point3DStd::Point3D& E1E2,
		const Point3DStd::Point3D& E2E1,
		double& distance,
		Point3DStd::Point3D& res);

	INTERFACE_API double Intersect_Ray3D_BoundingBox3D_plus(const Ray3DStd::Ray3D& ray, const Point3DStd::Point3D& min,
		const Point3DStd::Point3D& max, Point3DStd::Point3D& res);

	INTERFACE_API double Intersect_Ray3D_BoundingBox3D(const Ray3DStd::Ray3D& ray,
		const BoundingBox3DStd::BoundingBox3D& boundingBox, Point3DStd::Point3D& res);


	INTERFACE_API bool Intersect_Triangle3D_Plane3D_plus_plus(const Point3DStd::Point3D& t1, const Point3DStd::Point3D& t2, const Point3DStd::Point3D& t3,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& segStart, Point3DStd::Point3D& segEnd);

	INTERFACE_API bool ToDetermineWhetherTheReceiverCollidesWithTheRayByUsingTheCylindricalMethod(
		double curDistance,
		double rx_radius,
		const Point3DStd::Point3D& rayO,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& rxPoint);
}