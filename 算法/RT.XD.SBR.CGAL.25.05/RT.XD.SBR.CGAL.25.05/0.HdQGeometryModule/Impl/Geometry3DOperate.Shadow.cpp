


#include"../Input.h"

namespace Geometry3DOperateStd {


	/// <summary>
	/// 计算点在直线上的投影点
	/// </summary>
	/// <param name="p"></param>
	/// <param name="lineO"></param>
	/// <param name="lineVec"></param>
	/// <returns></returns>
	Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(const Point3DStd::Point3D& obj, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec) {
		
		if (Equals_Point3D(obj, lineO)) {
			return obj;
		}
		Point3DStd::Point3D pa = SubPoint3DPoint3D(obj, lineO);
		double lenpa = Length_Point3D(pa);
		double theta = GetAnglePoint3DPoint3D(lineVec, pa);

		Point3DStd::Point3D res;
		if (DotPoint3DPoint3D(lineVec, pa) > 0) {
			Point3DStd::Point3D temp_1 = MulDoublePoint3D(lenpa * cos(theta), lineVec);
			res = AddPoint3DPoint3D(temp_1, lineO);
		}
		else {
			Point3DStd::Point3D temp_1 = MulDoublePoint3D(-lenpa * cos(theta), lineVec);
			res = AddPoint3DPoint3D(temp_1, lineO);
		}
		return res;
	}

	/// <summary>
	///  计算点在直线段上的投影点，-1表示不在线段上，0表示在线段的端点上，1表示在线段内
	/// </summary>
	/// <param name="obj_location"></param>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="shadow_location"></param>
	/// <returns></returns>
	int CalculateShadowPoint3DOfPoint3DonLineSegment3D_plus_safe(
		const Point3DStd::Point3D& obj_location, 
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd, 
		Point3DStd::Point3D& shadow_location) {

		if (Geometry3DOperateStd::Equals_Point3D(obj_location, segStart)) {
			return 0;
		}
		if (Geometry3DOperateStd::Equals_Point3D(obj_location, segEnd)) {
			return 0;
		}
		Point3DStd::Point3D vec1 = SubPoint3DPoint3D(segEnd, segStart);
		Point3DStd::Point3D lineVec;
		if (!Geometry3DOperateStd::Normalization_Point3D_safe(vec1, lineVec)) {
			return -1;
		}
		shadow_location = CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(obj_location, segStart, lineVec);

		if (Location_Point3DonLineSegment3D_plus(shadow_location, segEnd, segStart)) {
			return 1;
		}
		return -1;
		
	}

	bool CalculateShadowPoint3DOfPoint3DonLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec, Point3DStd::Point3D& res) {
		if (IsZero(Length_Point3D(lineVec))) {
			return false;
		}
		res = CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p, lineO, lineVec);
		return true;
	}
	Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line) {
		return CalculateShadowPoint3DOfPoint3DonLine3D_plus_unsafe(p, line.p, line.vec);
	}

	bool CalculateShadowPoint3DOfPoint3DonLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line, Point3DStd::Point3D& res) {
		return CalculateShadowPoint3DOfPoint3DonLine3D_plus_safe(p, line.p, line.vec, res);
	}

	/// <summary>
	/// 计算点在射线上的投影点
	/// </summary>
	/// <param name="a"></param>
	/// <param name="ray"></param>
	/// <param name="a1"></param>
	/// <returns></returns>
	bool CalculateShadowPoint3DOfPoint3DonRay3D_safe(const Point3DStd::Point3D& a, const Ray3DStd::Ray3D& ray, Point3DStd::Point3D& res) {
		if (CalculateShadowPoint3DOfPoint3DonLine3D_plus_safe(a, ray.o, ray.vec, res)) {
			if (DotPoint3DPoint3D(ray.vec, SubPoint3DPoint3D(res, ray.o)) >= -1e9) {
				return true;
			}
		}
		return false;
	}
	
	/// <summary>
	/// 计算点在平面的投影点
	/// </summary>
	/// <param name="a"></param>
	/// <param name="p">平面上一点</param>
	/// <param name="n"></param>
	/// <returns></returns>
	Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(const Point3DStd::Point3D& a, const  Point3DStd::Point3D& p, const Point3DStd::Point3D& n) {
		double f = DotPoint3DPoint3D(n, SubPoint3DPoint3D(a, p));
		return SubPoint3DPoint3D(a, MulDoublePoint3D(f, n));
	}

	bool CalculateShadowPoint3DOfPoint3DonPlane3D_plus_safe(const Point3DStd::Point3D& a, const  Point3DStd::Point3D& p, const Point3DStd::Point3D& n, Point3DStd::Point3D& res) {
		if (IsZero(Length_Point3D(n))) {
			return false;
		}
		res = CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(a, p, n);
		return true;
	}

	Point3DStd::Point3D CalculateShadowPoint3DOfPoint3DonPlane3D_unsafe(const Point3DStd::Point3D& a, const  Plane3DStd::Plane3D& plane) {
		return CalculateShadowPoint3DOfPoint3DonPlane3D_plus_unsafe(a, plane.p, plane.n);
	}

	bool CalculateShadowPoint3DOfPoint3DonPlane3D_safe(const Point3DStd::Point3D& a, const  Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res) {
		return CalculateShadowPoint3DOfPoint3DonPlane3D_plus_safe(a, plane.p, plane.n, res);
	}

}