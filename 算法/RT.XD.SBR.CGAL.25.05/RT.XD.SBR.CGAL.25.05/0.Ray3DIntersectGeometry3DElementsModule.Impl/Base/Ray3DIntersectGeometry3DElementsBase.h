#pragma once

#include"../0.Ray3DIntersectGeometry3DElementsModule.Impl.Input.h"
#include<vector>
#include<set>



namespace Ray3DIntersectGeometry3DElementsBaseStd {

	/// <summary>
	/// 计算允许的最大误差
	/// </summary>
	const double Eps = 1e-9;

	/// <summary>
	/// 射线最大的运动距离
	/// </summary>
	const double RayMaxMovingDistance = 1e9;



}


namespace Ray3DIntersectGeometry3DElementsBaseStd {


	/// <summary>
	/// 计算向量长度
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	double Length_Point3D(const Point3D& p);


	/// <summary>
	/// 将一个向量放大或缩小
	/// </summary>
	/// <param name="d">zoom</param>
	/// <param name="p">向量</param>
	/// <returns>缩放后的向量</returns>
	Point3D MulDoublePoint3D(double d, const Point3D& p);


	/// <summary>
	/// 计算点的单位向量
	/// </summary>
	/// <param name="p">点</param>
	/// <returns>点的单位向量</returns>
	Point3D Normalization_Point3D(const Point3D& p);
}

namespace Ray3DIntersectGeometry3DElementsBaseStd {

	struct ExhaustTriangleAccelerateStruct {

		Point3D scenarioTriangleP1;
		Point3D scenarioTriangleN;
		Point3D scenarioE1;
		Point3D scenarioE2;
		Point3D scenarioE1E2;
		Point3D scenarioE2E1;

		

	};


}

namespace Ray3DIntersectGeometry3DElementsBaseStd {

	/// <summary>
	/// 计算一个非负数的结果是不是允许的误差
	/// </summary>
	/// <param name="d">不能是负数</param>
	/// <returns></returns>
	bool IsZero(double d);

	/// <summary>
	/// 计算一个数的绝对值的结果是不是允许的误差
	/// </summary>
	/// <param name="d"></param>
	/// <returns></returns>
	bool IsZeroAbs(double d);


	Point3D CreatePoint3D(double x, double y, double z);


	/// <summary>
	/// 计算两个点的差
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的差</returns>
	Point3D SubPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	/// <summary>
	 /// 点乘计算
	 /// </summary>
	 /// <param name="p1"></param>
	 /// <param name="p2"></param>
	 /// <returns></returns>
	double DotPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	/// <summary>
	/// 计算两个点的叉乘
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的叉乘</returns>
	Point3D CrossPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	bool Intersect_Ray3D_Triangle3D_plus(
		const Point3D& O,
		const Point3D& rayVec,
		const Point3D& V1,
		const Point3D& E1,
		const Point3D& E2,
		const Point3D& E1E2,
		const Point3D& E2E1,
		double& distance,
		Point3D& res);

}



namespace Ray3DIntersectGeometry3DElementsBaseStd {


	struct ExhaustCornerAccelerateStruct {

		double length;
		Point3D start;
		Point3D end;
		Point3D unit_vec;

		ExhaustCornerAccelerateStruct() {}

		~ExhaustCornerAccelerateStruct() {

		}

	};

	bool Equals_Point3D(const Point3D& p1, const Point3D& p2);


	/// <summary>
	/// 计算两个点的和
	/// </summary>
	/// <param name="p1">点1</param>
	/// <param name="p2">点2</param>
	/// <returns>两个点的和</returns>
	Point3D AddPoint3DPoint3D(const Point3D& p1, const Point3D& p2);

	/// <summary>
	/// 判断平面的法向量是否相同，180度或者0度都为真
	/// </summary>
	/// <param name="n1"></param>
	/// <param name="n2"></param>
	/// <param name="eps"></param>
	/// <returns></returns>
	bool Equals_Point3D_N(const Point3D& n1, const Point3D& n2);

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
		const Point3D& line2Vec);


	void AssignmentPoint3DPoint3D(Point3D& a, const Point3D& b);


	bool CrossPoint3DPoint3D_safe(const Point3D& p1, const Point3D& p2, Point3D& res);

	/// <summary>
	/// 计算点到直线的距离
	/// </summary>
	/// <param name="p"></param>
	/// <param name="line"></param>
	/// <param name="eps"></param>
	/// <param name="boundingBoxLength"></param>
	/// <returns></returns>
	double GetDistancePoint3DLine3D_plus_unsafe(const Point3D& p, const Point3D& lineO, const Point3D& lineVec);


	/// <summary>
	/// 获取两点之间的距离
	/// </summary>
	/// <param name="p1"></param>
	/// <param name="p2"></param>
	/// <returns></returns>
	double GetDistancePoint3DPoint3D(const Point3D& p1, const Point3D& p2);


	double CalMartix33(double num[3][3]);

	double GetDistanceLine3DLine3D_plus_unsafe(
		const Point3D& o1,
		const Point3D& d1,
		const Point3D& o2,
		const Point3D& d2,
		double& distance1,
		double& distance2,
		Point3D& res1,
		Point3D& res2);

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
		Point3D& res);

}

namespace Ray3DIntersectGeometry3DElementsBaseStd {

	void InitExhaustTriangleAccelerateStruct(
		const Scenario3D& scenario,
		std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs);


	void InitExhaustCornerAccelerateStruct(
		const Scenario3D& scenario,
		std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs);


}


namespace Ray3DIntersectGeometry3DElementsBaseStd {

	void CalculateRay3DIntersectTriangle3D(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<int>& triangleAccelerateStructIndex,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectTriangle3DExhaust(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectTriangle3DExhaustFirst(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		const std::set<int>& triangleAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectTriangle3DExhaustFirstAccelerateArray(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		int triangleAccelerateStructIndexSize,
		int* triangleAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DExhaust(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results);

	void CalculateRay3DIntersectCorner3DExhaustFirst(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DExhaustFirstAccelerate(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		const std::set<int>& cornerAccelerateStructIndex,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3DExhaustFirstAccelerateArray(
		const Point3D& ray_origin, const Point3D& ray_direction, double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		int cornerIndicesSize,
		int* cornerIndices,
		Ray3DIntersectGeometry3DElementResult& result);

	void CalculateRay3DIntersectCorner3D(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<int>& cornerAccelerateStructIndex,
		double cornerRadius,
		const std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct>& exhaustCornerAccelerateStructs,
		Ray3DIntersectGeometry3DElementsResults& results);
}