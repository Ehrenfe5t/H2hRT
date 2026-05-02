

#include"SotdpbeRay3DIntersectTriangle3DBallBvhTree.h"


#include <algorithm>
#include <memory>
#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <limits>
#include <chrono>
#include <fstream>
#include <list>

namespace SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd {

	struct SotdpbeExhaustTriangleAccelerateStruct {
		Point3D scenarioTriangleP1;
		Point3D scenarioTriangleN;
		Point3D scenarioE1;
		Point3D scenarioE2;
		Point3D scenarioE1E2;
		Point3D scenarioE2E1;
	};


	/// <summary>
	/// 计算允许的最大误差
	/// </summary>
	const double Eps = 1e-9;

	/// <summary>
	/// 射线最大的运动距离
	/// </summary>
	const double RayMaxMovingDistance = 1e9;

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

	void SotdpbeCalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(
		const Point3D& ray_origin, const Point3D& ray_direction,
		const std::vector<SotdpbeExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs,
		const std::set<int>& triangleAccelerateStructIndex,
		SotdpbeRay3DIntersectGeometry3DElementResult& result) {
		result.distance = RayMaxMovingDistance;
		result.elementIndex = -1;

		for (auto loop : triangleAccelerateStructIndex) {
			Point3D curRes;
			double curDis = RayMaxMovingDistance;
			if (Intersect_Ray3D_Triangle3D_plus(
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

	/// <summary>
	/// 计算向量长度
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	double Length_Point3D(const Point3D& p)
	{
		return sqrt(DotPoint3DPoint3D(p, p));
	}


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

	void InitExhaustTriangleAccelerateStruct(
		const Scenario3D& scenario,
		std::vector<SotdpbeExhaustTriangleAccelerateStruct>& exhaustTriangleAccelerateStructs) {

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

}


namespace SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd {

	// 精度
	double kAABBAccuracy = 0.01;

	// AABB盒
	struct AABB {
		// AABB盒表面积
		double Surface;
		Point3D Min;
		Point3D Max;
		// AABB盒几何元素元索引
		std::set<int> GeometryElementIndex;
	};


	// BvhNode
	struct AabbBvhTreeNode {

		// 该节点是否为叶子节点
		bool IsLeaf;

		double Radius;
		Point3D Center;
		// 左叶子节点
		std::unique_ptr<AabbBvhTreeNode> Left;
		// 右叶子节点
		std::unique_ptr<AabbBvhTreeNode> Right;
		// 该节点包含的AABB盒
		AABB Bound;

	};

	// Sah优化参数
	struct SahSplit {

		// 代价
		double BestCost;
		std::vector<int> LeftBoxesIndex;
		std::vector<int> RightBoxesIndex;
		AABB LeftBox;
		AABB RightBox;

	};


	/// <summary>
	/// 获取坐标轴
	/// </summary>
	/// <param name="p">待获取坐标点</param>
	/// <param name="axis">x=0,y=1,z=2</param>
	/// <returns></returns>
	double GetAxis(Point3D p, int axis) {
		switch (axis)
		{
		case 0:return p.x;
		case 1:return p.y;
		case 2:return p.z;
		default: break;
		}
		std::cout << "Error: Invalid axis" << std::endl;
		return 0.0;
	}

	double CalculateAABBSurfaceArea(const AABB& aabb) {

		double dx = aabb.Max.x - aabb.Min.x;
		double dy = aabb.Max.y - aabb.Min.y;
		double dz = aabb.Max.z - aabb.Min.z;

		// 若面元平行于某坐标平面，加厚AABB盒
		dx = fmax(kAABBAccuracy, dx);
		dy = fmax(kAABBAccuracy, dy);
		dz = fmax(kAABBAccuracy, dz);

		double surface = 0.02 * (dx * dy + dy * dz + dz * dx);
		return surface;
	}

	bool GetAABBByPoints(
		const std::list<Point3D>& points,
		AABB& aabb) {
		if (points.size() < 1) {
			return false;
		}

		aabb.Min = points.front();
		aabb.Max = points.front();

		for (auto& p : points) {

			aabb.Min.x = std::min(aabb.Min.x, p.x);
			aabb.Min.y = std::min(aabb.Min.y, p.y);
			aabb.Min.z = std::min(aabb.Min.z, p.z);

			aabb.Max.x = std::max(aabb.Max.x, p.x);
			aabb.Max.y = std::max(aabb.Max.y, p.y);
			aabb.Max.z = std::max(aabb.Max.z, p.z);

		}

		aabb.Min.x = aabb.Min.x - kAABBAccuracy;
		aabb.Min.y = aabb.Min.y - kAABBAccuracy;
		aabb.Min.z = aabb.Min.z - kAABBAccuracy;

		aabb.Max.x = aabb.Max.x + kAABBAccuracy;
		aabb.Max.y = aabb.Max.y + kAABBAccuracy;
		aabb.Max.z = aabb.Max.z + kAABBAccuracy;

		return true;
	}


	void CopySahSplit(const SahSplit& old, SahSplit& newSahSplit) {
		newSahSplit.BestCost = old.BestCost;
		{
			newSahSplit.LeftBoxesIndex.clear();
			newSahSplit.LeftBoxesIndex.resize(old.LeftBoxesIndex.size());
			for (int i = 0; i < old.LeftBoxesIndex.size(); ++i) {
				newSahSplit.LeftBoxesIndex[i] = old.LeftBoxesIndex[i];
			}
		}
		{
			newSahSplit.RightBoxesIndex.clear();
			newSahSplit.RightBoxesIndex.resize(old.RightBoxesIndex.size());
			for (int i = 0; i < old.RightBoxesIndex.size(); ++i) {
				newSahSplit.RightBoxesIndex[i] = old.RightBoxesIndex[i];
			}
		}

		{
			newSahSplit.LeftBox.Surface = old.LeftBox.Surface;
			newSahSplit.LeftBox.Min = old.LeftBox.Min;
			newSahSplit.LeftBox.Max = old.LeftBox.Max;
			newSahSplit.LeftBox.GeometryElementIndex.clear();
			for (auto index : old.LeftBox.GeometryElementIndex) {
				newSahSplit.LeftBox.GeometryElementIndex.insert(index);
			}
		}
		{
			newSahSplit.RightBox.Surface = old.RightBox.Surface;
			newSahSplit.RightBox.Min = old.RightBox.Min;
			newSahSplit.RightBox.Max = old.RightBox.Max;
			newSahSplit.RightBox.GeometryElementIndex.clear();
			for (auto index : old.RightBox.GeometryElementIndex) {
				newSahSplit.RightBox.GeometryElementIndex.insert(index);
			}
		}
	}


	bool FindValue(const std::set<int>& mySet, int value) {

		auto it = mySet.find(value);
		if (it != mySet.end()) {
			// 元素存在
			return true;
		}
		else {
			// 元素不存在
			return false;
		}
	}

	void CopyAABB(const AABB& old, AABB& newAABB) {

		newAABB.Surface = old.Surface;
		newAABB.Min = old.Min;
		newAABB.Max = old.Max;
		newAABB.GeometryElementIndex.clear();
		for (auto index : old.GeometryElementIndex) {
			newAABB.GeometryElementIndex.insert(index);
		}

	}


}

namespace SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd {

	std::vector<SotdpbeExhaustTriangleAccelerateStruct> globalExhaustTriangleAccelerateStructs;



	bool GetAABBByTriangle(
		const Scenario3D& scenario,
		int triangleIndex,
		AABB& aabb) {

		std::list<Point3D> points;
		{
			Triangle3D triangle = scenario.scenario_triangle3d_set[triangleIndex];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
			points.push_back(p1);
			points.push_back(p2);
			points.push_back(p3);
		}

		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}

		aabb.GeometryElementIndex.insert(triangleIndex);


		aabb.Surface = CalculateAABBSurfaceArea(aabb);

		return true;

	}

	bool GetAABBByTriangles(
		const Scenario3D& scenario,
		const std::vector<int>& indices,
		AABB& aabb) {

		std::list<Point3D> points;
		for (int i = 0; i < indices.size(); ++i) {
			int triangleIndex = indices[i];
			Triangle3D triangle = scenario.scenario_triangle3d_set[triangleIndex];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];
			points.push_back(p1);
			points.push_back(p2);
			points.push_back(p3);
		}

		if (!GetAABBByPoints(points, aabb)) {
			return false;
		}

		for (int triangleIndex : indices) {
			aabb.GeometryElementIndex.insert(triangleIndex);
		}

		aabb.Surface = indices.size() * CalculateAABBSurfaceArea(aabb);

		return true;

	}



	SahSplit CalculateSahSplitTriangle3D(
		const Scenario3D& scenario,
		const std::vector<int>& sortedIndices,
		int right_start_index) {

		std::vector<int> indices_left(right_start_index);
		for (int i = 0; i < right_start_index; ++i) {
			indices_left[i] = sortedIndices[i];
		}
		std::vector<int> indices_right(sortedIndices.size() - right_start_index);
		for (int i = right_start_index; i < sortedIndices.size(); ++i) {
			indices_right[i - right_start_index] = sortedIndices[i];
		}
		SahSplit split;

		if (!GetAABBByTriangles(scenario, indices_left, split.LeftBox)) {
			std::cout << "Error: GetAABBByTriangles(indices_left) failed" << std::endl;
		}

		if (!GetAABBByTriangles(scenario, indices_right, split.RightBox)) {
			std::cout << "Error: GetAABBByTriangles(indices_right) failed" << std::endl;
		}

		split.LeftBoxesIndex.clear();
		split.LeftBoxesIndex.resize(indices_left.size());
		for (int i = 0; i < indices_left.size(); ++i) {
			split.LeftBoxesIndex[i] = indices_left[i];
		}

		split.RightBoxesIndex.clear();
		split.RightBoxesIndex.resize(indices_right.size());
		for (int i = 0; i < indices_right.size(); ++i) {
			split.RightBoxesIndex[i] = indices_right[i];
		}


		split.BestCost = split.LeftBox.Surface + split.RightBox.Surface;

		return split;

	}


	void ChangeBoxMinByPoint(const Point3D& p, Point3D& min) {

		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		min.z = std::min(min.z, p.z);

	}

	void ChangeBoxMaxByPoint(const Point3D& p, Point3D& max) {
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
		max.z = std::max(max.z, p.z);
	}


	void ChangeTriangleBoxMinMax(
		int triangleIndex,
		const Scenario3D& scenario,
		Point3D& min,
		Point3D& max) {

		const Triangle3D& triangle = scenario.scenario_triangle3d_set[triangleIndex];
		const Point3D& p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
		const Point3D& p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
		const Point3D& p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];

		ChangeBoxMinByPoint(p1, min);
		ChangeBoxMaxByPoint(p1, max);

		ChangeBoxMinByPoint(p2, min);
		ChangeBoxMaxByPoint(p2, max);

		ChangeBoxMinByPoint(p3, min);
		ChangeBoxMaxByPoint(p3, max);

	}


	void ChangeCornerBoxMinMax(
		int cornerIndex,
		const Scenario3D& scenario,
		Point3D& min,
		Point3D& max) {

		const Corner3D& corner = scenario.scenario_corner3d_set[cornerIndex];
		const Point3D& p1 = scenario.scenario_point3d_set[corner.p1Index];
		const Point3D& p2 = scenario.scenario_point3d_set[corner.p2Index];

		ChangeBoxMinByPoint(p1, min);
		ChangeBoxMaxByPoint(p1, max);

		ChangeBoxMinByPoint(p2, min);
		ChangeBoxMaxByPoint(p2, max);

	}

	void FindBestSplitTriangle3DCore2(
		const Scenario3D& scenario,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {

		int left_size = (int)sortedIndices.size() - 1;

		std::vector<Point3D> left_split_min(left_size);
		std::vector<Point3D> left_split_max(left_size);

		std::vector<Point3D> right_split_min(left_size);
		std::vector<Point3D> right_split_max(left_size);

		{
			int index = 0;
			left_split_min[index].x = std::numeric_limits<double>::max();
			left_split_min[index].y = std::numeric_limits<double>::max();
			left_split_min[index].z = std::numeric_limits<double>::max();

			left_split_max[index].x = -std::numeric_limits<double>::max();
			left_split_max[index].y = -std::numeric_limits<double>::max();
			left_split_max[index].z = -std::numeric_limits<double>::max();

			ChangeTriangleBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);

			right_split_min[left_size - 1 - index].x = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].y = std::numeric_limits<double>::max();
			right_split_min[left_size - 1 - index].z = std::numeric_limits<double>::max();

			right_split_max[left_size - 1 - index].x = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].y = -std::numeric_limits<double>::max();
			right_split_max[left_size - 1 - index].z = -std::numeric_limits<double>::max();


			ChangeTriangleBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);

			index = 1;
			for (; index < left_size; ++index) {

				left_split_min[index].x = left_split_min[index - 1].x;
				left_split_min[index].y = left_split_min[index - 1].y;
				left_split_min[index].z = left_split_min[index - 1].z;

				left_split_max[index].x = left_split_max[index - 1].x;
				left_split_max[index].y = left_split_max[index - 1].y;
				left_split_max[index].z = left_split_max[index - 1].z;

				ChangeTriangleBoxMinMax(sortedIndices[index], scenario, left_split_min[index], left_split_max[index]);


				right_split_min[left_size - 1 - index].x = right_split_min[left_size - index].x;
				right_split_min[left_size - 1 - index].y = right_split_min[left_size - index].y;
				right_split_min[left_size - 1 - index].z = right_split_min[left_size - index].z;

				right_split_max[left_size - 1 - index].x = right_split_max[left_size - index].x;
				right_split_max[left_size - 1 - index].y = right_split_max[left_size - index].y;
				right_split_max[left_size - 1 - index].z = right_split_max[left_size - index].z;
				ChangeTriangleBoxMinMax(sortedIndices[left_size - index], scenario, right_split_min[left_size - 1 - index], right_split_max[left_size - 1 - index]);
			}
		}

		for (int i = 0; i < left_size; ++i) {
			left_split_min[i].x = left_split_min[i].x - kAABBAccuracy;
			left_split_min[i].y = left_split_min[i].y - kAABBAccuracy;
			left_split_min[i].z = left_split_min[i].z - kAABBAccuracy;

			left_split_max[i].x = left_split_max[i].x + kAABBAccuracy;
			left_split_max[i].y = left_split_max[i].y + kAABBAccuracy;
			left_split_max[i].z = left_split_max[i].z + kAABBAccuracy;

			right_split_min[i].x = right_split_min[i].x - kAABBAccuracy;
			right_split_min[i].y = right_split_min[i].y - kAABBAccuracy;
			right_split_min[i].z = right_split_min[i].z - kAABBAccuracy;

			right_split_max[i].x = right_split_max[i].x + kAABBAccuracy;
			right_split_max[i].y = right_split_max[i].y + kAABBAccuracy;
			right_split_max[i].z = right_split_max[i].z + kAABBAccuracy;
		}


		for (int i = 0; i < left_size; ++i) {
			int left_num = i + 1;
			int right_num = left_size - i;
			SahSplit split;
			split.LeftBox.Min = left_split_min[i];
			split.LeftBox.Max = left_split_max[i];
			split.LeftBox.Surface = left_num * CalculateAABBSurfaceArea(split.LeftBox);
			split.RightBox.Min = right_split_min[i];
			split.RightBox.Max = right_split_max[i];
			split.RightBox.Surface = right_num * CalculateAABBSurfaceArea(split.RightBox);
			split.BestCost = split.LeftBox.Surface + split.RightBox.Surface;


			if (split.BestCost < bestSplit.BestCost) {
				bestSplit.BestCost = split.BestCost;
				bestSplit.LeftBox.Surface = split.LeftBox.Surface;
				bestSplit.LeftBox.Min = split.LeftBox.Min;
				bestSplit.LeftBox.Max = split.LeftBox.Max;

				bestSplit.RightBox.Surface = split.RightBox.Surface;
				bestSplit.RightBox.Min = split.RightBox.Min;
				bestSplit.RightBox.Max = split.RightBox.Max;

				bestSplit.LeftBoxesIndex.clear();
				bestSplit.LeftBoxesIndex.resize(left_num);
				for (int j = 0; j < left_num; ++j) {
					bestSplit.LeftBoxesIndex[j] = sortedIndices[j];
				}

				bestSplit.RightBoxesIndex.clear();
				bestSplit.RightBoxesIndex.resize(right_num);
				for (int j = 0; j < right_num; ++j) {
					bestSplit.RightBoxesIndex[right_num - 1 - j] = sortedIndices[left_size - j];
				}


			}

		}

	}


	void FindBestSplitTriangle3DCore(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices,
		SahSplit& bestSplit) {


		for (int axis = 0; axis < 3; ++axis) {
			// 按当前轴对索引排序
			std::sort(sortedIndices.begin(), sortedIndices.end(),
				[&](int a, int b) { return GetAxis(center_points[a], axis) < GetAxis(center_points[b], axis); });

			FindBestSplitTriangle3DCore2(scenario, sortedIndices, bestSplit);

		}

	}

	SahSplit FindBestSplitTriangle3D(
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& sortedIndices)
	{


		SahSplit bestSplit;
		bestSplit.BestCost = std::numeric_limits<double>::max();

		if (center_points.size() < 1 || sortedIndices.size() < 1) {
			return bestSplit;
		}
		else if (sortedIndices.size() == 1) {

			AABB leftBox;
			if (!GetAABBByTriangle(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}

			bestSplit.BestCost = leftBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}
		else if (sortedIndices.size() == 2) {
			AABB leftBox;
			AABB rightBox;

			if (!GetAABBByTriangle(scenario, sortedIndices[0], leftBox)) {
				return bestSplit;
			}

			if (!GetAABBByTriangle(scenario, sortedIndices[1], rightBox)) {
				return bestSplit;
			}


			bestSplit.BestCost = leftBox.Surface + rightBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.GeometryElementIndex.clear();
				for (auto index : leftBox.GeometryElementIndex) {
					bestSplit.LeftBox.GeometryElementIndex.insert(index);
				}

			}
			{
				bestSplit.RightBoxesIndex.push_back(sortedIndices[1]);
				bestSplit.RightBox.Min = rightBox.Min;
				bestSplit.RightBox.Max = rightBox.Max;
				bestSplit.RightBox.Surface = rightBox.Surface;
				bestSplit.RightBox.GeometryElementIndex.clear();
				for (auto index : rightBox.GeometryElementIndex) {
					bestSplit.RightBox.GeometryElementIndex.insert(index);
				}

			}
			return bestSplit;
		}

		FindBestSplitTriangle3DCore(scenario, center_points, sortedIndices, bestSplit);

		return bestSplit;
	}

	/// <summary>
	/// 递归构建AabbBvhTree
	/// </summary>
	/// <param name="aabbs">所有圆柱体的AABB数组</param>
	/// <param name="centroids">所有圆柱体的质心坐标数组</param>
	/// <param name="indices">当前待处理的圆柱体索引列表</param>
	/// <param name="depth">当前递归深度</param>
	/// <param name="maxLeafSize">叶子节点最大圆柱体数量</param>
	/// <param name="maxDepth">最大递归深度</param>
	/// <returns>构建好的 BvhNode_Cor 节点</returns>
	std::unique_ptr<AabbBvhTreeNode> BuildAabbBvhTreeRecursiveTriangle3D(
		int depth,
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario,
		const std::vector<Point3D>& center_points,
		std::vector<int>& indices) {

		auto root_node = std::make_unique<AabbBvhTreeNode>();
		root_node->IsLeaf = false;
		if (!GetAABBByTriangles(scenario, indices, root_node->Bound)) {
			return std::unique_ptr<AabbBvhTreeNode>();
		}
		root_node->Center.x = 0.5 * (root_node->Bound.Min.x + root_node->Bound.Max.x);
		root_node->Center.y = 0.5 * (root_node->Bound.Min.y + root_node->Bound.Max.y);
		root_node->Center.z = 0.5 * (root_node->Bound.Min.z + root_node->Bound.Max.z);
		{
			double dx = root_node->Bound.Max.x - root_node->Bound.Min.x;
			double dy = root_node->Bound.Max.y - root_node->Bound.Min.y;
			double dz = root_node->Bound.Max.z - root_node->Bound.Min.z;
			root_node->Radius = 0.5 * sqrt(dx * dx + dy * dy + dz * dz);
		}
		// 终止条件：叶子节点
		if (indices.size() <= maxLeafSize || depth >= maxDepth) {
			root_node->IsLeaf = true;
			return root_node;
		}


		// 寻找最优分割
		SahSplit split = FindBestSplitTriangle3D(scenario, center_points, indices);


		// 递归构建子树
		if (!split.LeftBoxesIndex.empty()) {
			root_node->Left = BuildAabbBvhTreeRecursiveTriangle3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.LeftBoxesIndex);
		}

		if (!split.RightBoxesIndex.empty()) {
			root_node->Right = BuildAabbBvhTreeRecursiveTriangle3D(depth + 1, maxLeafSize, maxDepth, scenario, center_points, split.RightBoxesIndex);
		}

		return root_node;
	}

	std::unique_ptr<AabbBvhTreeNode> BuildTriangle3DAabbBvhTreeNode(
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario) {
		// 准备数据

		std::vector<Point3D> center_points(scenario.trianglesCount);
		for (int i = 0; i < scenario.trianglesCount; ++i) {

			Triangle3D& triangle = scenario.scenario_triangle3d_set[i];
			Point3D p1 = scenario.scenario_point3d_set[triangle.triangleP1Index];
			Point3D p2 = scenario.scenario_point3d_set[triangle.triangleP2Index];
			Point3D p3 = scenario.scenario_point3d_set[triangle.triangleP3Index];

			center_points[i].x = (p1.x + p2.x + p3.x) / 3.0;
			center_points[i].y = (p1.y + p2.y + p3.y) / 3.0;
			center_points[i].z = (p1.z + p2.z + p3.z) / 3.0;

		}

		// 创建初始索引列表
		std::vector<int> indices(center_points.size());
		for (int i = 0; i < center_points.size(); ++i) {
			indices[i] = i;
		}

		// 递归构建Tri-BVH-Tree
		return BuildAabbBvhTreeRecursiveTriangle3D(0, maxLeafSize, maxDepth, scenario, center_points, indices);
	}




	bool Ray3DIntersectBall(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const Point3D& center,
		double radius) {

		Point3D op;
		op.x = ray_origin.x - center.x;
		op.y = ray_origin.y - center.y;
		op.z = ray_origin.z - center.z;

		double length_op = sqrt(op.x * op.x + op.y * op.y + op.z * op.z);

		if (length_op < radius) {
			return true;
		}
		double dot = op.x * ray_direction.x + op.y * ray_direction.y + op.z * ray_direction.z;
		if (dot > 0) {
			return false;
		}


		double temp_1 = length_op * length_op;
		double h_radius = sqrt(temp_1 - dot * dot);
		if (h_radius < radius) {
			//没有达到接受条件
			return true;
		}
		else {
			return false;
		}
	}

	void Ray3DIntersectAabbBvhTreeTriangle3D2(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const std::unique_ptr<AabbBvhTreeNode>& root,
		std::set<int>& triangleIndices) {

		if (!root) {
			return;
		}

		bool isIntersect = Ray3DIntersectBall(ray_origin, ray_direction, root->Center, root->Radius);

		if (!isIntersect) {
			return;
		}

		if (root->IsLeaf) {

			for (auto triangleIndex : root->Bound.GeometryElementIndex) {
				triangleIndices.insert(triangleIndex);
			}
		}


		Ray3DIntersectAabbBvhTreeTriangle3D2(ray_origin, ray_direction, root->Left, triangleIndices);
		Ray3DIntersectAabbBvhTreeTriangle3D2(ray_origin, ray_direction, root->Right, triangleIndices);



	}


}



namespace SotdpbeRay3DIntersectTriangle3DBallBvhTreeStd {

	std::unique_ptr<AabbBvhTreeNode> Triangle3DAabbBvhTree;

	void InitializeAabbBvhTree(const Scenario3D& scenario) {

		InitExhaustTriangleAccelerateStruct(scenario, globalExhaustTriangleAccelerateStructs);
		Triangle3DAabbBvhTree = std::move(BuildTriangle3DAabbBvhTreeNode(1, 13, scenario));
	}

	void CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		SotdpbeRay3DIntersectGeometry3DElementResult& result) {

		std::set<int> triangleIndices;

		Ray3DIntersectAabbBvhTreeTriangle3D2(ray_origin, ray_direction, Triangle3DAabbBvhTree, triangleIndices);

		SotdpbeCalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(ray_origin, ray_direction, globalExhaustTriangleAccelerateStructs, triangleIndices, result);
	}


}



