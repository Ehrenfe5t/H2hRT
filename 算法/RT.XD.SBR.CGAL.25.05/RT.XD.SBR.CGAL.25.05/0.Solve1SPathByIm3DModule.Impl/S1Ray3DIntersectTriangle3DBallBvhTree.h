#pragma once

#include "0.Solve1SPathByIm3DModule.Impl.Input.h"


/// <summary>
/// 当前全局常量
/// </summary>
namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {
	const double Eps = 1e-9;
	const double RayMaxMovingDistance = 1e9;
	const double C = 299792458.0;
	const double Pi = 3.141592653589793238462643383279;
}

namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {

	double GetAxis(Point3D p, int axis);

	bool IsZero(double d);

	bool IsZeroAbs(double d);

	Point3D SubPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	double Length_Point3D(const Point3D& p);

	Point3D MulDoublePoint3D(double d, const Point3D& p);

	Point3D Normalization_Point3D(const Point3D& p);

	bool Equals_Point3D(const Point3D& p1, const Point3D& p2);

	Point3D AddPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	double GetDistancePoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	double GetDistanceLine3DLine3D(const Point3D& o1, const Point3D& d1,
		const Point3D& o2, const Point3D& d2,
		double& distance1, double& distance2, Point3D& res1, Point3D& res2);

	bool Intersect_Ray3D_Triangle3D_Plus(const Point3D& O, const Point3D& rayVec,
		const Point3D& V1,
		const Point3D& E1,
		const Point3D& E2,
		const Point3D& E1E2,
		const Point3D& E2E1,
		double& distance, Point3D& res);


	bool Ray3DIntersectBall(const Point3D& ray_origin, const Point3D& ray_direction,
		const Point3D& center, double radius);


	double GetAnglePoint3DPoint3D(const Point3D& a, const Point3D& b);

	double GetThetaI(const Point3D& v1, const Point3D& n1);

}

namespace S1Ray3DIntersectTriangle3DBallBvhTreeStd {

	struct Triangle3DMaterial
	{

		int materialIndex1 = 0;
		int materialIndex2 = 0;
		double roughness = 0.0;

		Point3D scenarioTriangleP1;
		Point3D scenarioTriangleP2;
		Point3D scenarioTriangleP3;
		Point3D scenarioTriangleN;
		Point3D scenarioE1;
		Point3D scenarioE2;
		Point3D scenarioE1E2;
		Point3D scenarioE2E1;

	};

	struct Corner3DMaterial
	{
		int materialIndex11 = 0;
		int materialIndex12 = 0;
		int materialIndex21 = 0;
		int materialIndex22 = 0;
		double phiE = 0.0;
		double length = 0.0;
		Point3D cornerStart;
		Point3D cornerEnd;
		Point3D cornerN1;
		Point3D cornerN2;
		Point3D cornerT1p3;
		Point3D cornerT2p3;

		Point3D o;
		Point3D x;
		Point3D y;
		Point3D z;
	};


	struct Ray3DIntersectGeometry3DResult
	{
		/// <summary>
		/// 碰撞类型，0表示和三角形碰撞了，1表示和边碰撞了
		/// </summary>
		int type = -1;
		/// <summary>
		/// 碰撞的元素编号
		/// </summary>
		int elementIndex = -1;
		double distance = 1.0e19;
		Point3D location;
	};

	void InitializeScenario3D(bool switchCorner, double cornerRadius, long long frequency, const Scenario3D& scenario, const MaterialSet& materialSet);

	bool Ray3DIntersectTriangle3D(int triangle_index, const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result);

	void Ray3DIntersectTriangle3Ds(const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result);

	void Ray3DIntersectTriangle3Ds_sorted(int pre_number, const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result);

	void Ray3DsIntersectTriangle3Ds_sorted(int pre_number, const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<std::vector<Ray3DIntersectGeometry3DResult>>& result);

	void Ray3DIntersectTriangle3DsFirst(const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result);

	void Ray3DsIntersectTriangle3DsFirst(const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result);

	bool Ray3DIntersectCorner3D(int corner_index, const Point3D& ray_origin, const Point3D& ray_direction, Ray3DIntersectGeometry3DResult& result);

	void Ray3DIntersectCorner3Ds(const Point3D& ray_origin, const Point3D& ray_direction, std::vector<Ray3DIntersectGeometry3DResult>& result);

	void Ray3DsIntersectCorner3Ds(const std::vector<Point3D>& ray_origin, const std::vector<Point3D>& ray_direction, std::vector<std::vector<Ray3DIntersectGeometry3DResult>>& result);

	bool CalculateCanSeeNode_Plus(
		double cornerRadius,
		const Point3D& segStart,
		const Point3D& segEnd,
		Point3D& lineVec);

	bool CalculateCanSeeNode(
		double cornerRadius,
		const Point3D& segStart,
		const Point3D& segEnd);
}