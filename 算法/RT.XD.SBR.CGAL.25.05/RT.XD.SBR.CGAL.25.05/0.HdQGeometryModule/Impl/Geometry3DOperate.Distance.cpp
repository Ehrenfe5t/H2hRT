

#include"../Input.h"


namespace Geometry3DOperateStd {


	/// <summary>
	/// 获取两点之间的距离
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <returns></returns>
	double GetDistancePoint3DPoint3D(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2)
	{
		Point3DStd::Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		return Length_Point3D(temp111);
	}

	/// <summary>
	/// 计算点到直线的距离
	/// </summary>
	/// <param name="p"></param>
	/// <param name="line"></param>
	/// <param name="eps"></param>
	/// <param name="boundingBoxLength"></param>
	/// <returns></returns>
	double GetDistancePoint3DLine3D_plus_unsafe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec)
	{
		if (Equals_Point3D(p, lineO))
		{
			return 0.0;
		}
		Point3DStd::Point3D b = AddPoint3DPoint3D(lineO, lineVec);
		if (Equals_Point3D(p, b))
		{
			return 0.0;
		}
		Point3DStd::Point3D pa = SubPoint3DPoint3D(lineO, p);
		Point3DStd::Point3D pb = SubPoint3DPoint3D(b, p);
		if (Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(pa, pb)) {
			return 0.0;
		}

		Point3DStd::Point3D temp111;
		if (!CrossPoint3DPoint3D_safe(pa, pb, temp111)) {
			return GlobalConstantStd::BoundingBoxLength;
		}
		double s = Length_Point3D(temp111);
		double ab = Length_Point3D(lineVec);
		return s / ab;
	}

	bool GetDistancePoint3DLine3D_plus_safe(const Point3DStd::Point3D& p, const Point3DStd::Point3D& lineO, const Point3DStd::Point3D& lineVec, double& distance) {
		if (IsZero(Length_Point3D(lineVec))) {
			return false;
		}
		distance = GetDistancePoint3DLine3D_plus_unsafe(p, lineO, lineVec);
		return true;
	}

	double GetDistancePoint3DLine3D_unsafe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line) {
		return GetDistancePoint3DLine3D_plus_unsafe(p, line.p, line.vec);
	}

	bool GetDistancePoint3DLine3D_safe(const Point3DStd::Point3D& p, const Line3DStd::Line3D& line, double& distance) {
		return GetDistancePoint3DLine3D_plus_safe(p, line.p, line.vec, distance);
	}


	double GetDistancePoint3DRay3D_unsafe(const Point3DStd::Point3D& p, const Ray3DStd::Ray3D& ray)
	{
		auto vec2 = SubPoint3DPoint3D(p, ray.o);
		if (DotPoint3DPoint3D(vec2, ray.vec) > 0) {
			return GetDistancePoint3DLine3D_plus_unsafe(p, ray.o, ray.vec);
		}
		return GetDistancePoint3DPoint3D(p, ray.o);
	}

	bool GetDistancePoint3DRay3D_safe(const Point3DStd::Point3D& p, const Ray3DStd::Ray3D& ray, double& distance)
	{
		if (IsZero(Length_Point3D(ray.vec))) {
			return false;
		}
		distance = GetDistancePoint3DRay3D_unsafe(p, ray);
		return true;
	}

	double GetDistanceLine3DLine3D_plus_unsafe(
		const Point3DStd::Point3D& o1,
		const Point3DStd::Point3D& d1,
		const Point3DStd::Point3D& o2,
		const Point3DStd::Point3D& d2,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2) {
		Point3DStd::Point3D d3 = CrossPoint3DPoint3D(d1, d2);
		Point3DStd::Point3D o = SubPoint3DPoint3D(o2, o1);
		double D[3][3];
		D[0][0] = d1.x;
		D[1][0] = d1.y;
		D[2][0] = d1.z;
		D[0][1] = -d2.x;
		D[1][1] = -d2.y;
		D[2][1] = -d2.z;
		D[0][2] = d3.x;
		D[1][2] = d3.y;
		D[2][2] = d3.z;

		double D1[3][3];
		D1[0][0] = o.x;
		D1[1][0] = o.y;
		D1[2][0] = o.z;
		D1[0][1] = -d2.x;
		D1[1][1] = -d2.y;
		D1[2][1] = -d2.z;
		D1[0][2] = d3.x;
		D1[1][2] = d3.y;
		D1[2][2] = d3.z;
		double D2[3][3];
		D2[0][0] = d1.x;
		D2[1][0] = d1.y;
		D2[2][0] = d1.z;
		D2[0][1] = o.x;
		D2[1][1] = o.y;
		D2[2][1] = o.z;
		D2[0][2] = d3.x;
		D2[1][2] = d3.y;
		D2[2][2] = d3.z;
		double D3[3][3];
		D3[0][0] = d1.x;
		D3[1][0] = d1.y;
		D3[2][0] = d1.z;
		D3[0][1] = -d2.x;
		D3[1][1] = -d2.y;
		D3[2][1] = -d2.z;
		D3[0][2] = o.x;
		D3[1][2] = o.y;
		D3[2][2] = o.z;

		double martixD = MathOperateStd::CalMartix33(D);
		if (IsZeroAbs(martixD)) {
			return GlobalConstantStd::BoundingBoxLength;
		}
		double martixD1 = MathOperateStd::CalMartix33(D1);
		double martixD2 = MathOperateStd::CalMartix33(D2);
		//double martixD3 = CalMartix33(D3);
		double t1 = martixD1 / martixD;
		double t2 = martixD2 / martixD;
		//double k = martixD3 / martixD;
		Point3DStd::Point3D p1 = AddPoint3DPoint3D(o1, MulDoublePoint3D(t1, d1));
		Point3DStd::Point3D p2 = AddPoint3DPoint3D(o2, MulDoublePoint3D(t2, d2));
		AssignmentPoint3DPoint3D(res1, p1);
		AssignmentPoint3DPoint3D(res2, p2);
		return GetDistancePoint3DPoint3D(p1, p2);
	}


	double GetDistanceLine3DLine3D_unsafe(
		const Line3DStd::Line3D& line1,
		const Line3DStd::Line3D& line2,
		Point3DStd::Point3D& res1,
		Point3DStd::Point3D& res2) {
		return GetDistanceLine3DLine3D_plus_unsafe(line1.p, line1.vec, line2.p, line2.vec, res1, res2);
	}


	double GetDistancePoint3DPlane3D_plus_unsafe(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& planep,
		const Point3DStd::Point3D& planen)
	{
		if (Equals_Point3D(p, planep))
		{
			return 0.0;
		}
		Point3DStd::Point3D po = SubPoint3DPoint3D(planep, p);
		if (IsZeroAbs(DotPoint3DPoint3D(po, planen)))
		{
			//std::cout << "1096 GetDistancePoint3DPlane 角度较小几乎与平面垂直\n";
			return 0.0;
		}

		double theta = GetAnglePoint3DPoint3D_unsafe(po, planen);
		return Length_Point3D(po) * cos(theta);
	}

	bool GetDistancePoint3DPlane3D_plus_safe(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& planep,
		const Point3DStd::Point3D& planen,double &distance)
	{
		if (IsZero(Length_Point3D(planen))) {
			distance = GlobalConstantStd::BoundingBoxLength;
			return false;
		}
		distance  = GetDistancePoint3DPlane3D_plus_unsafe(p, planep, planen);
		return true;
	}

	/// <summary>
	/// 计算点到平面的距离
	/// </summary>
	/// <param name="p"></param>
	/// <param name="plane"></param>
	/// <param name="boundingBoxLength">这个距离表示无穷远</param>
	/// <returns>点到平面的距离</returns>
	double GetDistancePoint3DPlane3D_unsafe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane)
	{
		return GetDistancePoint3DPlane3D_plus_unsafe(p, plane.p, plane.n);
	}
	bool GetDistancePoint3DPlane3D_safe(const Point3DStd::Point3D& p, const Plane3DStd::Plane3D& plane, double& distance)
	{
		return GetDistancePoint3DPlane3D_plus_safe(p, plane.p, plane.n, distance);
	}

	double GetDistancePoint3DLineSegment3D_plus(
		const Point3DStd::Point3D& p,
		const Point3DStd::Point3D& segStart,
		const Point3DStd::Point3D& segEnd) {
		Point3DStd::Point3D vec = SubPoint3DPoint3D(segEnd, segStart);
		double length = Length_Point3D(vec);
		if (IsZero(length)) {
			return GetDistancePoint3DPoint3D(p,segStart);
		}
		vec = MulDoublePoint3D(1.0 / length, vec);
		double dis1 = GetDistancePoint3DLine3D_plus_unsafe(p, segStart, vec);
		double dis2 = GetDistancePoint3DPoint3D(p, segStart);
		double dis3 = GetDistancePoint3DPoint3D(p, segEnd);
		if (dis1 > dis2) {
			dis1 = dis2;
		}
		if (dis1 > dis3) {
			dis1 = dis3;
		}
		return dis1;
	}


	/// <summary>
	/// 计算点到线段的距离
	/// </summary>
	/// <param name="p"></param>
	/// <param name="line"></param>
	/// <param name="eps"></param>
	/// <param name="boundingBoxLength"></param>
	/// <param name="shadowVertex3D"></param>
	/// <returns></returns>
	double GetDistancePoint3DLineSegment3D(
		const Point3DStd::Point3D& p,
		const LineSegment3DStd::LineSegment3D& seg)
	{
		return  GetDistancePoint3DLineSegment3D_plus(p,seg.start,seg.end);
	}
	

}
