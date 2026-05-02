
#include "Ray3DIntersectGeometry3DElementsBase.h"

#include<math.h>
#include<iostream>

namespace Ray3DIntersectGeometry3DElementsBaseStd {

	/// <summary>
	/// 计算一个非负数的结果是不是允许的误差
	/// </summary>
	/// <param name="d">不能是负数</param>
	/// <returns></returns>
	bool IsZero(double d) {
		if (d <= Eps)
		{
			return true;
		}
		return false;
	}

	/// <summary>
	/// 计算一个数的绝对值的结果是不是允许的误差
	/// </summary>
	/// <param name="d"></param>
	/// <returns></returns>
	bool IsZeroAbs(double d) {
		return IsZero(abs(d));
	}


	Point3D CreatePoint3D(double x, double y, double z) {
		Point3D res;
		res.x = x;
		res.y = y;
		res.z = z;
		return res;
	}


	/// <summary>
	/// 计算两个点的差
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的差</returns>
	Point3D SubPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return CreatePoint3D(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
	}

	/// <summary>
	 /// 点乘计算
	 /// </summary>
	 /// <param name="p1"></param>
	 /// <param name="p2"></param>
	 /// <returns></returns>
	double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
	}

	/// <summary>
	/// 计算两个点的叉乘
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的叉乘</returns>
	Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		double l = p1.x;
		double m = p1.y;
		double n = p1.z;
		double o = p2.x;
		double p = p2.y;
		double q = p2.z;
		Point3D res = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);

		return res;
	}

	bool Intersect_Ray3D_Triangle3D_plus(
		const Point3D& O,
		const Point3D& rayVec,
		const Point3D& V1,
		const Point3D& E1,
		const Point3D& E2,
		const Point3D& E1E2,
		const Point3D& E2E1,
		double& distance,
		Point3D& res) {

		distance = RayMaxMovingDistance;

		Point3D D = CreatePoint3D(-rayVec.x, -rayVec.y, -rayVec.z);

		Point3D T = SubPoint3DPoint3D(V1, O);
		double f1 = DotPoint3DPoint3D(D, E2E1);
		if (IsZeroAbs(f1)) {
			//除数不能是0
			return false;
		}

		double f2 = DotPoint3DPoint3D(T, E1E2);
		double t = f2 / f1;

		//此时交点在射线负轴上
		if (t < Eps)
		{
			return false;
		}

		Point3D P = CrossPoint3DPoint3D(D, E2);

		double f3 = DotPoint3DPoint3D(P, T);
		double u = f3 / f1;
		if (u < 0.0)
		{
			//在三角形的平面内但不在三角形内
			return false;
		}
		Point3D Q = CrossPoint3DPoint3D(T, E1);
		double f4 = DotPoint3DPoint3D(Q, D);
		double v = f4 / f1;
		if (v < 0.0 || u + v > 1.0f)
		{
			//在三角形的平面内但不在三角形内
			return false;
		}
		distance = t;
		{
			res.x = O.x + t * rayVec.x;
			res.y = O.y + t * rayVec.y;
			res.z = O.z + t * rayVec.z;
		}
		return true;
	}

}



namespace Ray3DIntersectGeometry3DElementsBaseStd {


	/// <summary>
	/// 计算向量长度
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	double Length_Point3D(const Point3D& p)
	{
		return sqrt(DotPoint3DPoint3D(p, p));
	}


	/// <summary>
	/// 将一个向量放大或缩小
	/// </summary>
	/// <param name="d">zoom</param>
	/// <param name="p">向量</param>
	/// <returns>缩放后的向量</returns>
	Point3D MulDoublePoint3D(double d, const Point3D& p)
	{
		return CreatePoint3D(d * p.x, d * p.y, d * p.z);
	}


	/// <summary>
	/// 计算点的单位向量
	/// </summary>
	/// <param name="p">点</param>
	/// <returns>点的单位向量</returns>
	Point3D Normalization_Point3D(const Point3D& p)
	{
		double len = Length_Point3D(p);
		if (len < Eps * Eps)
		{
			std::cout << "Point3DNormalization传入了非法参数" << std::endl;
			return CreatePoint3D(0.0, 0.0, 0.0);
		}
		return MulDoublePoint3D(1 / len, p);
	}
}




namespace Ray3DIntersectGeometry3DElementsBaseStd {

	bool Equals_Point3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		double dis = Length_Point3D(temp111);
		if (dis <= Eps)
		{
			return true;
		}
		return false;
	}


	/// <summary>
	/// 计算两个点的和
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的和</returns>
	Point3D AddPoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		return CreatePoint3D(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
	}

	/// <summary>
	/// 判断平面的法向量是否相同，180度或者0度都为真
	/// </summary>
	/// <param name="n1"></param>
	/// <param name="n2"></param>
	/// <param name="eps"></param>
	/// <returns></returns>
	bool Equals_Point3D_N(const Point3D& n1, const Point3D& n2)
	{
		if (Equals_Point3D(n1, n2))
		{
			return true;
		}
		Point3D n = AddPoint3DPoint3D(n1, n2);

		double length = Length_Point3D(n);

		if (length <= Eps)
		{
			return true;
		}
		return false;
	}

	/// <summary>
	/// 两个向量是否平行或者共线
	/// </summary>
	/// <param name="line1O"></param>
	/// <param name="line1Vec"></param>
	/// <param name="line2O"></param>
	/// <param name="line2Vec"></param>
	/// <returns></returns>
	bool Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(
		const Point3D& line1Vec,
		const Point3D& line2Vec) {
		return Equals_Point3D_N(line1Vec, line2Vec);
	}


	void AssignmentPoint3DPoint3D(Point3D& a, const Point3D& b) {
		a.x = b.x;
		a.y = b.y;
		a.z = b.z;
	}


	bool CrossPoint3DPoint3D_safe(const Point3D& p1, const Point3D& p2, Point3D& res)
	{
		double l = p1.x;
		double m = p1.y;
		double n = p1.z;
		double o = p2.x;
		double p = p2.y;
		double q = p2.z;
		Point3D res2 = CreatePoint3D(m * q - n * p, n * o - l * q, l * p - m * o);
		if (Length_Point3D(res2) > 0) {
			AssignmentPoint3DPoint3D(res, res2);
			return true;
		}
		return false;
	}

	/// <summary>
	/// 计算点到直线的距离
	/// </summary>
	/// <param name="p"></param>
	/// <param name="line"></param>
	/// <param name="eps"></param>
	/// <param name="boundingBoxLength"></param>
	/// <returns></returns>
	double GetDistancePoint3DLine3D_plus_unsafe(const Point3D& p, const Point3D& lineO, const Point3D& lineVec)
	{
		if (Equals_Point3D(p, lineO))
		{
			return 0.0;
		}
		Point3D b = AddPoint3DPoint3D(lineO, lineVec);
		if (Equals_Point3D(p, b))
		{
			return 0.0;
		}
		Point3D pa = SubPoint3DPoint3D(lineO, p);
		Point3D pb = SubPoint3DPoint3D(b, p);
		if (Location_ParallelOrCoincident_Point3D_Point3D_plus_unsafe(pa, pb)) {
			return 0.0;
		}

		Point3D temp111;
		if (!CrossPoint3DPoint3D_safe(pa, pb, temp111)) {
			return RayMaxMovingDistance;
		}
		double s = Length_Point3D(temp111);
		double ab = Length_Point3D(lineVec);
		return s / ab;
	}


	/// <summary>
	/// 获取两点之间的距离
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <returns></returns>
	double GetDistancePoint3DPoint3D(const Point3D& p1, const Point3D& p2)
	{
		Point3D temp111 = SubPoint3DPoint3D(p1, p2);
		return Length_Point3D(temp111);
	}


	double CalMartix33(double num[3][3]) {
		return num[0][0] * num[1][1] * num[2][2] +
			num[0][1] * num[1][2] * num[2][0] +
			num[0][2] * num[1][0] * num[2][1] -
			num[2][0] * num[1][1] * num[0][2] -
			num[2][1] * num[1][2] * num[0][0] -
			num[1][0] * num[0][1] * num[2][2];
	}

	double GetDistanceLine3DLine3D_plus_unsafe(
		const Point3D& o1,
		const Point3D& d1,
		const Point3D& o2,
		const Point3D& d2,
		double& distance1,
		double& distance2,
		Point3D& res1,
		Point3D& res2) {
		Point3D d3 = CrossPoint3DPoint3D(d1, d2);
		Point3D o = SubPoint3DPoint3D(o2, o1);
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

		double martixD = CalMartix33(D);
		if (IsZeroAbs(martixD)) {
			return RayMaxMovingDistance;
		}
		double martixD1 = CalMartix33(D1);
		double martixD2 = CalMartix33(D2);
		//double martixD3 = CalMartix33(D3);
		double t1 = martixD1 / martixD;
		double t2 = martixD2 / martixD;
		//double k = martixD3 / martixD;
		Point3D p1 = AddPoint3DPoint3D(o1, MulDoublePoint3D(t1, d1));
		Point3D p2 = AddPoint3DPoint3D(o2, MulDoublePoint3D(t2, d2));
		AssignmentPoint3DPoint3D(res1, p1);
		AssignmentPoint3DPoint3D(res2, p2);
		distance1 = t1;
		distance2 = t2;
		return GetDistancePoint3DPoint3D(p1, p2);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="radiusCorner"></param>
	/// <param name="O"></param>
	/// <param name="rayVec"></param>
	/// <param name="seg_start">线段的起点</param>
	/// <param name="seg_end">线段的终点</param>
	/// <param name="seg_length">线段的长度</param>
	/// <param name="seg_unit_vec">线段从起点到终点的单位向量</param>
	/// <param name="rayLength"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool Intersect_Ray3D_Corner3D(
		double radiusCorner,
		const Point3D& O,
		const Point3D& rayVec,
		const Point3D& seg_start,
		const Point3D& seg_end,
		double seg_length,
		const Point3D& seg_unit_vec,
		double& rayLength,
		Point3D& res) {

		Point3D res1, res2;
		//先计算端点到线段的距离，不能太小
		double pToSeg = GetDistancePoint3DLine3D_plus_unsafe(O, seg_start, seg_unit_vec);
		if (pToSeg < Eps) {
			return false;
		}

		double distance1 = RayMaxMovingDistance;
		double distance2 = RayMaxMovingDistance;

		double curSegSegDis = GetDistanceLine3DLine3D_plus_unsafe(
			O,
			rayVec,
			seg_start,
			seg_unit_vec,
			distance1,
			distance2,
			res1,
			res2);

		if (curSegSegDis <= radiusCorner) {

			if (distance1 < Eps) {
				//在射线的另一方向
				return false;
			}
			if (distance2 < Eps) {
				//不在线段上
				return false;
			}

			if (distance2 > seg_length) {
				//不在线段上
				return false;
			}

			//这里的碰撞点是线段上的点，因此，碰撞距离需要重新计算
			rayLength = GetDistancePoint3DPoint3D(O, res2);
			res = res2;
			return true;
		}


		return false;
	}

}


namespace Ray3DIntersectGeometry3DElementsBaseStd {


	void InitExhaustTriangleAccelerateStruct(
		const Scenario3D& scenario,
		std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs) {

		exhaustTriangleAccelerateStructs.clear();
		exhaustTriangleAccelerateStructs.resize(scenario.trianglesCount);

		for (int loop = 0; loop < scenario.trianglesCount; ++loop) {
			Point3D p1 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[scenario.scenario_triangle3d_set[loop].triangleP3Index];
			Point3D n = Normalization_Point3D(CrossPoint3DPoint3D(
				SubPoint3DPoint3D(p2, p1), SubPoint3DPoint3D(p3, p1)));
			if (DotPoint3DPoint3D(n, scenario.scenario_triangle3d_set[loop].n) < 0.0) {
				auto temp = p2;
				p2 = p3;
				p3 = temp;
			}

			Point3D E1 = SubPoint3DPoint3D(p1, p2);
			Point3D E2 = SubPoint3DPoint3D(p1, p3);
			Point3D E1E2 = CrossPoint3DPoint3D(E1, E2);
			Point3D E2E1 = CrossPoint3DPoint3D(E2, E1);

			exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1 = p1;
			exhaustTriangleAccelerateStructs[loop].scenarioTriangleN = scenario.scenario_triangle3d_set[loop].n;
			exhaustTriangleAccelerateStructs[loop].scenarioE1 = E1;
			exhaustTriangleAccelerateStructs[loop].scenarioE2 = E2;
			exhaustTriangleAccelerateStructs[loop].scenarioE1E2 = E1E2;
			exhaustTriangleAccelerateStructs[loop].scenarioE2E1 = E2E1;

		}

	}


	void InitExhaustCornerAccelerateStruct(
		const Scenario3D& scenario,
		std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs) {


		exhaustCornerAccelerateStructs.clear();
		exhaustCornerAccelerateStructs.resize(scenario.cornersCount);

		for (int loop = 0; loop < scenario.cornersCount; ++loop) {
			Point3D start = scenario.scenario_point3d_set[scenario.scenario_corner3d_set[loop].p1Index];
			Point3D end = scenario.scenario_point3d_set[scenario.scenario_corner3d_set[loop].p2Index];

			Point3D vec = SubPoint3DPoint3D(end, start);
			double length = Length_Point3D(vec);
			Point3D unit_vec = MulDoublePoint3D(1.0 / length, vec);

			exhaustCornerAccelerateStructs[loop].start = start;
			exhaustCornerAccelerateStructs[loop].end = end;
			exhaustCornerAccelerateStructs[loop].unit_vec = unit_vec;
			exhaustCornerAccelerateStructs[loop].length = length;

		}
	}

}



namespace Ray3DIntersectGeometry3DElementsBaseStd {

	void CalculateRay3DIntersectTriangle3D(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<int>& triangleAccelerateStructIndex,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results) {

		for (int i = 0; i < triangleAccelerateStructIndex.size(); ++i) {
			int loop = triangleAccelerateStructIndex[i];
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Triangle3D_plus(
				ray_origin,
				ray_direction,
				exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
				exhaustTriangleAccelerateStructs[loop].scenarioE1,
				exhaustTriangleAccelerateStructs[loop].scenarioE2,
				exhaustTriangleAccelerateStructs[loop].scenarioE1E2,
				exhaustTriangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {

				Ray3DIntersectGeometry3DElementResult rayCollisionGeometricElementResult;
				rayCollisionGeometricElementResult.type = 0;
				rayCollisionGeometricElementResult.distance = curDis;
				rayCollisionGeometricElementResult.elementIndex = loop;
				rayCollisionGeometricElementResult.location = curRes;
				results.results.emplace_back(rayCollisionGeometricElementResult);
			}

		}
	}

	void CalculateRay3DIntersectTriangle3DExhaust(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results) {


		for (int loop = 0; loop < exhaustTriangleAccelerateStructs.size(); ++loop) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Triangle3D_plus(
				ray_origin,
				ray_direction,
				exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
				exhaustTriangleAccelerateStructs[loop].scenarioE1,
				exhaustTriangleAccelerateStructs[loop].scenarioE2,
				exhaustTriangleAccelerateStructs[loop].scenarioE1E2,
				exhaustTriangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {

				Ray3DIntersectGeometry3DElementResult rayCollisionGeometricElementResult;
				rayCollisionGeometricElementResult.type = 0;
				rayCollisionGeometricElementResult.distance = curDis;
				rayCollisionGeometricElementResult.elementIndex = loop;
				rayCollisionGeometricElementResult.location = curRes;
				results.results.emplace_back(rayCollisionGeometricElementResult);
			}

		}
	}


	void CalculateRay3DIntersectTriangle3DExhaustFirst(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (int loop = 0; loop < exhaustTriangleAccelerateStructs.size(); ++loop) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Triangle3D_plus(
				ray_origin,
				ray_direction,
				exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
				exhaustTriangleAccelerateStructs[loop].scenarioE1,
				exhaustTriangleAccelerateStructs[loop].scenarioE2,
				exhaustTriangleAccelerateStructs[loop].scenarioE1E2,
				exhaustTriangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {


				//std::cout << "loop:" << loop << std::endl;

				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}

			}

		}
	}


	void CalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		const std::set<int>& triangleAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (auto loop : triangleAccelerateStructIndex) {


			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Triangle3D_plus(
				ray_origin,
				ray_direction,
				exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
				exhaustTriangleAccelerateStructs[loop].scenarioE1,
				exhaustTriangleAccelerateStructs[loop].scenarioE2,
				exhaustTriangleAccelerateStructs[loop].scenarioE1E2,
				exhaustTriangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {


				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}

			}

		}
	}


	void CalculateRay3DIntersectTriangle3DExhaustFirstAccelerateArray(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		int triangleAccelerateStructIndexSize,
		int* triangleAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (int i=0; i<triangleAccelerateStructIndexSize; ++i) {

			int loop = triangleAccelerateStructIndex[i];

			//std::cout << "loop:" << loop << std::endl;
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Triangle3D_plus(
				ray_origin,
				ray_direction,
				exhaustTriangleAccelerateStructs[loop].scenarioTriangleP1,
				exhaustTriangleAccelerateStructs[loop].scenarioE1,
				exhaustTriangleAccelerateStructs[loop].scenarioE2,
				exhaustTriangleAccelerateStructs[loop].scenarioE1E2,
				exhaustTriangleAccelerateStructs[loop].scenarioE2E1,
				curDis,
				curRes)) {


				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}

			}

		}
	}
	

	void CalculateRay3DIntersectCorner3DExhaustFirst(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (int loop = 0; loop < exhaustCornerAccelerateStructs.size(); ++loop) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Corner3D(
				cornerRadius,
				ray_origin,
				ray_direction,
				exhaustCornerAccelerateStructs[loop].start,
				exhaustCornerAccelerateStructs[loop].end,
				exhaustCornerAccelerateStructs[loop].length,
				exhaustCornerAccelerateStructs[loop].unit_vec,
				curDis,
				curRes)) {

				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}
			}

		}

	}

	void CalculateRay3DIntersectCorner3DExhaustFirstAccelerate(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		const std::set<int>& cornerAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (auto loop : cornerAccelerateStructIndex) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Corner3D(
				cornerRadius,
				ray_origin,
				ray_direction,
				exhaustCornerAccelerateStructs[loop].start,
				exhaustCornerAccelerateStructs[loop].end,
				exhaustCornerAccelerateStructs[loop].length,
				exhaustCornerAccelerateStructs[loop].unit_vec,
				curDis,
				curRes)) {

				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}
			}

		}

	}


	void CalculateRay3DIntersectCorner3DExhaustFirstAccelerateArray(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		int cornerIndicesSize,
		int* cornerIndices,
		Ray3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (int i = 0; i < cornerIndicesSize; ++i) {
			int loop = cornerIndices[i];
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Corner3D(
				cornerRadius,
				ray_origin,
				ray_direction,
				exhaustCornerAccelerateStructs[loop].start,
				exhaustCornerAccelerateStructs[loop].end,
				exhaustCornerAccelerateStructs[loop].length,
				exhaustCornerAccelerateStructs[loop].unit_vec,
				curDis,
				curRes)) {

				if (result.distance > curDis) {

					result.type = 0;
					result.distance = curDis;
					result.elementIndex = loop;
					result.location = curRes;

				}
			}

		}

	}
	

	void CalculateRay3DIntersectCorner3DExhaust(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results) {

		for (int loop = 0; loop < exhaustCornerAccelerateStructs.size(); ++loop) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Corner3D(
				cornerRadius,
				ray_origin,
				ray_direction,
				exhaustCornerAccelerateStructs[loop].start,
				exhaustCornerAccelerateStructs[loop].end,
				exhaustCornerAccelerateStructs[loop].length,
				exhaustCornerAccelerateStructs[loop].unit_vec,
				curDis,
				curRes)) {

				Ray3DIntersectGeometry3DElementResult rayCollisionGeometricElementResult;
				rayCollisionGeometricElementResult.type = 1;
				rayCollisionGeometricElementResult.distance = curDis;
				rayCollisionGeometricElementResult.elementIndex = loop;
				rayCollisionGeometricElementResult.location = curRes;
				results.results.emplace_back(rayCollisionGeometricElementResult);
			}

		}

	}


	void CalculateRay3DIntersectCorner3D(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<int>& cornerAccelerateStructIndex,
		double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results) {

		for (int i = 0; i < cornerAccelerateStructIndex.size(); ++i) {
			int loop = cornerAccelerateStructIndex[i];
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Ray3DIntersectGeometry3DElementsBaseStd::Intersect_Ray3D_Corner3D(
				cornerRadius,
				ray_origin,
				ray_direction,
				exhaustCornerAccelerateStructs[loop].start,
				exhaustCornerAccelerateStructs[loop].end,
				exhaustCornerAccelerateStructs[loop].length,
				exhaustCornerAccelerateStructs[loop].unit_vec,
				curDis,
				curRes)) {

				Ray3DIntersectGeometry3DElementResult rayCollisionGeometricElementResult;
				rayCollisionGeometricElementResult.type = 1;
				rayCollisionGeometricElementResult.distance = curDis;
				rayCollisionGeometricElementResult.elementIndex = loop;
				rayCollisionGeometricElementResult.location = curRes;
				results.results.emplace_back(rayCollisionGeometricElementResult);
			}

		}

	}
}
