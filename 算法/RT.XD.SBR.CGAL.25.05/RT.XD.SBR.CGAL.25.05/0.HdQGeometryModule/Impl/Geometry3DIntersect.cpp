
#include"../Input.h"

namespace Geometry3DIntersectStd {

	//点对点、直线、线段、射线、平面、三角形


	bool Intersect_Point3D_Point3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2) {
		return Geometry3DOperateStd::Equals_Point3D(p1, p2);
	}

	bool Intersect_Point3D_Line3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec) {
		return Geometry3DOperateStd::Location_Point3DonLine3D_plus_unsafe(p, lineO, lineVec);
	}


	bool Intersect_Point3D_Ray3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec) {
		Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(p, rayO);
		double dot = Geometry3DOperateStd::DotPoint3DPoint3D(vec, rayVec);
		if (dot < GlobalConstantStd::Eps) {
			return false;
		}

		if (Geometry3DOperateStd::Location_Point3DonLine3D_plus_unsafe(p, rayO, rayVec)) {
			
			return true;
		}
		return false;
	}

	//直线

	bool Intersect_Line3D_Line3D_plus_plus_base(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		double eps,
		double& distance,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2) {

		distance = Geometry3DOperateStd::GetDistanceLine3DLine3D_plus_unsafe(
			line1O, line1Vec, line2O, line2Vec, res1, res2);

		if (distance <= eps) {
			return true;
		}
		return false;
	}


	bool Intersect_Line3D_Line3D_plus_plus_2(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		double eps,
		Point3DStd::Point3D& res) {
		Point3DStd::Point3D res1, res2;
		double distance = Geometry3DOperateStd::GetDistanceLine3DLine3D_plus_unsafe(
			line1O, line1Vec, line2O, line2Vec, res1, res2);

		if (distance <= eps) {
			res = Geometry3DOperateStd::Center_Point3D_Point3D(res1, res2);
			return true;
		}
		return false;
	}

	bool Intersect_Line3D_Line3D_plus_plus_1(
		const Point3DStd::Point3D& line1O, const Point3DStd::Point3D& line1Vec,
		const Point3DStd::Point3D& line2O, const Point3DStd::Point3D& line2Vec,
		Point3DStd::Point3D& res) {
		return Intersect_Line3D_Line3D_plus_plus_2(line1O, line1Vec,
			line2O, line2Vec, GlobalConstantStd::Eps, res);
	}

	bool Intersect_Line3D_LineSegment3D_plus_plus_2(
		const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		double eps,
		Point3DStd::Point3D& res) {
		Point3DStd::Point3D res1, res2;
		Point3DStd::Point3D line2O = segStart;
		Point3DStd::Point3D line2Vec1 = Geometry3DOperateStd::SubPoint3DPoint3D(segEnd, segStart);
		Point3DStd::Point3D line2Vec;
		if (!Geometry3DOperateStd::Normalization_Point3D_safe(line2Vec1, line2Vec)) {
			if (Intersect_Point3D_Line3D_plus_unsafe(line2O, lineO, lineVec)) {
				res = line2O;
				return true;
			}
			return false;
		}
		double distance = Geometry3DOperateStd::GetDistanceLine3DLine3D_plus_unsafe(
			lineO, lineVec, line2O, line2Vec, res1, res2);

		if (distance <= eps) {

			if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(res2, segStart, segEnd)) {
				res = res1;
				return true;
			}

		}
		return false;
	}

	bool Intersect_Line3D_LineSegment3D_plus_plus_1(
		const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		Point3DStd::Point3D& res) {
		return Intersect_Line3D_LineSegment3D_plus_plus_2(lineO, lineVec, segStart, segEnd, GlobalConstantStd::Eps, res);
	}

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
	int Intersect_Ray3D_LineSegment3D_plus_plus_2(
		const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		double eps,
		double& distance,
		Point3DStd::Point3D& res) {
		
		if (Intersect_Line3D_LineSegment3D_plus_plus_2(rayO, rayVec, segStart, segEnd, eps, res)) {
			if (Geometry3DOperateStd::Equals_Point3D(rayO,res)) {
				return 0;
			}
			Point3DStd::Point3D vec = Geometry3DOperateStd::SubPoint3DPoint3D(res, rayO);
			double dot = Geometry3DOperateStd::DotPoint3DPoint3D(vec, rayVec);
			if (dot < GlobalConstantStd::Eps) {
				return -1;
			}
			distance = Geometry3DOperateStd::Length_Point3D(vec);
			return 1;
		}

		return -1;
	}

	/// <summary>
	/// -1射线和线段无交点；0表示交点在射线端点；1表示有一个在射线上的交点，并且不在射线端点上
	/// </summary>
	/// <param name="rayO"></param>
	/// <param name="rayVec"></param>
	/// <param name="segStart"></param>
	/// <param name="segEnd"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	int Intersect_Ray3D_LineSegment3D_plus_plus_1(
		const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		Point3DStd::Point3D& res) {
		double distance;
		return  Intersect_Ray3D_LineSegment3D_plus_plus_2(rayO, rayVec,segStart, segEnd,GlobalConstantStd::Eps, distance, res);
	}

	/// <summary>
	/// 直线和平面交互，true表示有一个焦点，false表示平行或者共线
	/// </summary>
	/// <param name="lineO"></param>
	/// <param name="lineVec"></param>
	/// <param name="planeO"></param>
	/// <param name="planeN"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool Intersect_Line3D_Plane3D_plus_plus(const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res) {

		double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(lineVec, planeN);
		//平行或者共线
		if (MathOperateStd::OneNumberIsZeroByEps(d1) == 1) {
			return false;
		}
		//直线方向向量为vec(m,n,p)，经过点p(x0,y0,z0),(x-x0)/m=(y-y0)/n=(z-z0)/p=t,x = m*t+x0,y=n*t+y0,
		//平面公式为a*(x-x0)+b*(y-y0)+c*(z-z0)=0，其定义为与固定点p(x0,y0,z0)的连线垂直于固定方向n（a,b,c）的所有的点的集合
		//直线L：(x-a)/m=(x-b)/n=(z-c)/p和空间平面π：Ax+By+Cz+D=0;
		//x=mt+a；y=nt+b；z=pt+c；
		//t=-(Aa+Bb+Cc+D)/(Am+Bn+Cp)
		//在这里p就是(a,b,c)，vec(m,n,p),n(A,B,C)
		auto pp = Geometry3DOperateStd::SubPoint3DPoint3D(planeO, lineO);
		double t = (Geometry3DOperateStd::DotPoint3DPoint3D(pp, planeN)) / d1;
		//x=mt+a；y=nt+b；z=pt+c
		Point3DStd::Point3D temp111 = Geometry3DOperateStd::MulDoublePoint3D(t, lineVec);
		Point3DStd::Point3D temp222 = Geometry3DOperateStd::AddPoint3DPoint3D(temp111, lineO);
		Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, temp222);
		return true;

	}

	bool Intersect_Line3D_Plane3D_plus_plus_d(const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res, double& distance) {

		double d1 = Geometry3DOperateStd::DotPoint3DPoint3D(lineVec, planeN);
		//平行或者共线
		if (MathOperateStd::OneNumberIsZeroByEps(d1) == 1) {
			return false;
		}
		//直线方向向量为vec(m,n,p)，经过点p(x0,y0,z0),(x-x0)/m=(y-y0)/n=(z-z0)/p=t,x = m*t+x0,y=n*t+y0,
		//平面公式为a*(x-x0)+b*(y-y0)+c*(z-z0)=0，其定义为与固定点p(x0,y0,z0)的连线垂直于固定方向n（a,b,c）的所有的点的集合
		//直线L：(x-a)/m=(x-b)/n=(z-c)/p和空间平面π：Ax+By+Cz+D=0;
		//x=mt+a；y=nt+b；z=pt+c；
		//t=-(Aa+Bb+Cc+D)/(Am+Bn+Cp)
		//在这里p就是(a,b,c)，vec(m,n,p),n(A,B,C)
		auto pp = Geometry3DOperateStd::SubPoint3DPoint3D(planeO, lineO);
		double t = (Geometry3DOperateStd::DotPoint3DPoint3D(pp, planeN)) / d1;
		//x=mt+a；y=nt+b；z=pt+c
		Point3DStd::Point3D temp111 = Geometry3DOperateStd::MulDoublePoint3D(t, lineVec);
		Point3DStd::Point3D temp222 = Geometry3DOperateStd::AddPoint3DPoint3D(temp111, lineO);
		Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, temp222);
		distance = abs(t);
		return true;

	}

	/// <summary>
	/// 直线和平面交互，true表示有一个焦点，false表示平行或者共线
	/// </summary>
	/// <param name="line"></param>
	/// <param name="plane"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool Intersect_Line3D_Plane3D(const Line3DStd::Line3D& line, const Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res) {
		return Intersect_Line3D_Plane3D_plus_plus(line.p, line.vec, plane.p, plane.n, res);
	}

	//射线对点、线段、平面、三角形



	bool Intersect_LineSegment3D_Plane3D_plus_plus(const Point3DStd::Point3D& segStart, const Point3DStd::Point3D& segEnd,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res) {

		Point3DStd::Point3D vec1 = Geometry3DOperateStd::SubPoint3DPoint3D(segEnd, segStart);
		Point3DStd::Point3D vec;
		if (!Geometry3DOperateStd::Normalization_Point3D_safe(vec1, vec)) {
			return false;
		}
		if (Intersect_Line3D_Plane3D_plus_plus(segStart, vec, planeO, planeN, res)) {
			if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(res, segStart, segEnd)) {
				return true;
			}
		}
		return false;
	}

	bool Intersect_LineSegment3D_LineSegment3D_plus(const Point3DStd::Point3D& seg1Start, const Point3DStd::Point3D& seg1End,
		const Point3DStd::Point3D& seg2Start, const Point3DStd::Point3D& seg2End) {

		if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(seg2Start, seg1Start, seg1End)) {
			return true;
		}
		if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(seg2End, seg1Start, seg1End)) {
			return true;
		}
		if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(seg1Start, seg2Start, seg2End)) {
			return true;
		}
		if (Geometry3DOperateStd::Location_Point3DonLineSegment3D_plus(seg1End, seg2Start, seg2End)) {
			return true;
		}
		return false;
	}


	bool Intersect_Triangle3D_Plane3D_plus_plus(const Point3DStd::Point3D& t1, const Point3DStd::Point3D& t2, const Point3DStd::Point3D& t3,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& segStart, Point3DStd::Point3D& segEnd) {

		Point3DStd::Point3D p1, p2, p3;
		bool state1 = Intersect_LineSegment3D_Plane3D_plus_plus(t1, t2, planeO, planeN, p1);
		bool state2 = Intersect_LineSegment3D_Plane3D_plus_plus(t2, t3, planeO, planeN, p2);
		bool state3 = Intersect_LineSegment3D_Plane3D_plus_plus(t3, t1, planeO, planeN, p3);

		if (state1 && state2 && !state3) {
			if (Geometry3DOperateStd::Equals_Point3D(p1, p2)) {
				return false;
			}
			segStart = p1;
			segEnd = p2;
			return true;
		}
		else if (state1 && !state2 && state3) {
			if (Geometry3DOperateStd::Equals_Point3D(p1, p3)) {
				return false;
			}
			segStart = p1;
			segEnd = p3;
			return true;
		}
		else if (!state1 && state2 && state3) {
			if (Geometry3DOperateStd::Equals_Point3D(p3, p2)) {
				return false;
			}
			segStart = p2;
			segEnd = p3;
			return true;
		}
		return false;
	}

	/// <summary>
	/// 射线和平面交互，true表示有一个焦点，false表示平行或者共线或者没有焦点
	/// </summary>
	/// <param name="rayO"></param>
	/// <param name="rayVec"></param>
	/// <param name="planeO"></param>
	/// <param name="planeN"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool Intersect_Ray3D_Plane3D_plus_plus(const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res) {

		if (Intersect_Line3D_Plane3D_plus_plus(rayO, rayVec, planeO, planeN, res)) {
			auto vec = Geometry3DOperateStd::SubPoint3DPoint3D(res, rayO);
			if (Geometry3DOperateStd::DotPoint3DPoint3D(vec, rayVec) > 0) {
				return true;
			}
		}
		return false;
	}


	bool Intersect_Ray3D_Plane3D_plus_plus_d(const Point3DStd::Point3D& rayO, const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& planeO, const Point3DStd::Point3D& planeN, Point3DStd::Point3D& res,double& distance) {

		if (Intersect_Line3D_Plane3D_plus_plus_d(rayO, rayVec, planeO, planeN, res,distance)) {
			auto vec = Geometry3DOperateStd::SubPoint3DPoint3D(res, rayO);
			if (Geometry3DOperateStd::DotPoint3DPoint3D(vec, rayVec) > 0) {
				return true;
			}
		}
		return false;
	}

	/// <summary>
	/// 射线和平面交互，true表示有一个焦点，false表示平行或者共线或者没有焦点
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="plane"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool Intersect_Ray3D_Plane3D(const Ray3DStd::Ray3D& ray, const Plane3DStd::Plane3D& plane, Point3DStd::Point3D& res) {
		return Intersect_Ray3D_Plane3D_plus_plus(ray.o, ray.vec, plane.p, plane.n, res);
	}

	/// <summary>
	/// 射线和三角形碰撞，1表示只有一个交点，2表示没有交点但射线和三角形所在的平面有焦点
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="trianglep1"></param>
	/// <param name="E1"></param>
	/// <param name="E2"></param>
	/// <param name="distance"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	int Intersect_Ray3D_Triangle3D_plus2(
		const Point3DStd::Point3D& rayO,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& trianglep1, 
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2, 
		double& distance, 
		Point3DStd::Point3D& res) {

		distance = GlobalConstantStd::BoundingBoxLength;

		Point3DStd::Point3D D(-rayVec.x, -rayVec.y, -rayVec.z);
		Point3DStd::Point3D P = Geometry3DOperateStd::CrossPoint3DPoint3D(D, E2);
		double f1 = Geometry3DOperateStd::DotPoint3DPoint3D(P, E1);
		if (Geometry3DOperateStd::IsZeroAbs(f1)) {
			return -1;
		}
		Point3DStd::Point3D T = Geometry3DOperateStd::SubPoint3DPoint3D(trianglep1, rayO);
		Point3DStd::Point3D Q = Geometry3DOperateStd::CrossPoint3DPoint3D(T, E1);
		double f2 = Geometry3DOperateStd::DotPoint3DPoint3D(Q, E2);
		double t = f2 / f1;
		//此时交点在射线负轴上
		if (t < 1e-12)
		{
			return 0;
		}
		double f3 = Geometry3DOperateStd::DotPoint3DPoint3D(P, T);
		double u = f3 / f1;
		if (u < 0.0)
		{
			return 2;
		}
		double f4 = Geometry3DOperateStd::DotPoint3DPoint3D(Q, D);
		double v = f4 / f1;
		if (v < 0.0 || u + v > 1.0f)
		{
			return 2;
		}
		distance = t;
		Point3DStd::Point3D temp111 = Geometry3DOperateStd::MulDoublePoint3D(t, rayVec);
		Point3DStd::Point3D temp222 = Geometry3DOperateStd::AddPoint3DPoint3D(rayO, temp111);
		Geometry3DOperateStd::AssignmentPoint3DPoint3D(res, temp222);
		return 1;
	}


	int Intersect_Ray3D_Triangle3D_plus(
		const Point3DStd::Point3D& O,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& V1,
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2,
		const Point3DStd::Point3D& E1E2,
		const Point3DStd::Point3D& E2E1,
		double& distance,
		Point3DStd::Point3D& res) {

		distance = GlobalConstantStd::BoundingBoxLength;

		Point3DStd::Point3D D(-rayVec.x, -rayVec.y, -rayVec.z);
		Point3DStd::Point3D T = Geometry3DOperateStd::SubPoint3DPoint3D(V1, O);
		double f1 = Geometry3DOperateStd::DotPoint3DPoint3D(D, E2E1);
		if (Geometry3DOperateStd::IsZeroAbs(f1)) {
			return -1;
		}

		double f2 = Geometry3DOperateStd::DotPoint3DPoint3D(T, E1E2);
		double t = f2 / f1;

		//此时交点在射线负轴上
		if (t < 1e-12)
		{
			return 0;
		}

		Point3DStd::Point3D P = Geometry3DOperateStd::CrossPoint3DPoint3D(D, E2);
		
		double f3 = Geometry3DOperateStd::DotPoint3DPoint3D(P, T);
		double u = f3 / f1;
		if (u < 0.0)
		{
			return 2;
		}
		Point3DStd::Point3D Q = Geometry3DOperateStd::CrossPoint3DPoint3D(T, E1);
		double f4 = Geometry3DOperateStd::DotPoint3DPoint3D(Q, D);
		double v = f4 / f1;
		if (v < 0.0 || u + v > 1.0f)
		{
			return 2;
		}
		distance = t;
		{
			res.x = O.x + t * rayVec.x;
			res.y = O.y + t * rayVec.y;
			res.z = O.z + t * rayVec.z;
		}
		return 1;
	}


	int Intersect_Ray3D_Triangle3D(
		const Ray3DStd::Ray3D& ray,
		const Point3DStd::Point3D& V1,
		const Point3DStd::Point3D& E1,
		const Point3DStd::Point3D& E2,
		const Point3DStd::Point3D& E1E2,
		const Point3DStd::Point3D& E2E1,
		double& distance, 
		Point3DStd::Point3D& res) {
		return Intersect_Ray3D_Triangle3D_plus(
			ray.o,
			ray.vec,
			V1,
			E1,
			E2,
			E1E2,
			E2E1,
			distance,
			res);
	}


	/// <summary>
	/// 没有测试入参，计算射线和边界框的碰撞结果，返回距离
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	double Intersect_Ray3D_BoundingBox3D_plus(const Ray3DStd::Ray3D& ray, const Point3DStd::Point3D& min,
		const Point3DStd::Point3D& max, Point3DStd::Point3D& res) {

		double t = GlobalConstantStd::BoundingBoxLength;

		if (ray.vec.x > GlobalConstantStd::MaxEps) {
			double t2 = (max.x - ray.o.x) / ray.vec.x;
			if (t2 < t) {
				t = t2;
			}
		}

		if (ray.vec.x < -GlobalConstantStd::MaxEps) {
			double t2 = (min.x - ray.o.x) / ray.vec.x;
			if (t2 < t) {
				t = t2;
			}
		}

		if (ray.vec.y > GlobalConstantStd::MaxEps) {
			double t2 = (max.y - ray.o.y) / ray.vec.y;
			if (t2 > 0 && t2 < t) {
				t = t2;
			}
		}

		if (ray.vec.y < -GlobalConstantStd::MaxEps) {
			double t2 = (min.y - ray.o.y) / ray.vec.y;
			if (t2 > 0 && t2 < t) {
				t = t2;
			}
		}

		if (ray.vec.z > GlobalConstantStd::MaxEps) {
			double t2 = (max.z - ray.o.z) / ray.vec.z;
			if (t2 > 0 && t2 < t) {
				t = t2;
			}
		}

		if (ray.vec.z < -GlobalConstantStd::MaxEps) {
			double t2 = (min.z - ray.o.z) / ray.vec.z;
			if (t2 > 0 && t2 < t) {
				t = t2;
			}
		}

		res.x = ray.o.x + t * ray.vec.x;
		res.y = ray.o.y + t * ray.vec.y;
		res.z = ray.o.z + t * ray.vec.z;
		return t;
	}

	/// <summary>
	/// 没有测试入参，计算射线和边界框的碰撞结果，返回距离
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="boundingBox"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	double Intersect_Ray3D_BoundingBox3D(const Ray3DStd::Ray3D& ray,
		const BoundingBox3DStd::BoundingBox3D& boundingBox, Point3DStd::Point3D& res) {
		return Intersect_Ray3D_BoundingBox3D_plus(ray, boundingBox.min, boundingBox.max, res);
	}


	/// <summary>
	/// 以圆柱法去判断接收机是否与射线碰撞，及100米和1000米处的接受半径是一样的
	/// </summary>
	/// <param name="curDistance"></param>
	/// <param name="rx_max_radius"></param>
	/// <param name="rayO"></param>
	/// <param name="rayVec"></param>
	/// <param name="rxPoint"></param>
	/// <returns></returns>
	bool ToDetermineWhetherTheReceiverCollidesWithTheRayByUsingTheCylindricalMethod(
		double curDistance,
		double rx_radius,
		const Point3DStd::Point3D& rayO,
		const Point3DStd::Point3D& rayVec,
		const Point3DStd::Point3D& rxPoint) {
		Point3DStd::Point3D op = Geometry3DOperateStd::SubPoint3DPoint3D(rxPoint, rayO);
		double dot_rayVec_op = Geometry3DOperateStd::DotPoint3DPoint3D(op, rayVec);
		//判断方向
		if (dot_rayVec_op < GlobalConstantStd::Eps) {
			return false;
		}
		double txAndRxDis = Geometry3DOperateStd::Length_Point3D(op);
		if (txAndRxDis >= curDistance) {
			//说明被遮挡
			return false;
		}

		double disRxAndLineP = Geometry3DOperateStd::GetDistancePoint3DPoint3D(rayO, rxPoint);
		if (disRxAndLineP < 0.005) {
			//没有达到接受条件，射线端点与接收天线太近，剔除
			return false;
		}

		double temp_1 = txAndRxDis * txAndRxDis;
		double h_radius = sqrt(temp_1 - dot_rayVec_op * dot_rayVec_op);
		//圆柱法,当前版本只支持圆柱法，圆锥法不在支持
		if (h_radius > rx_radius) {
			//没有达到接受条件
			return false;
		}

		return true;
	}
}