

#include"Ray3DIntersectGeometry3DElementsAabbBvhTree3.h"

#include"../Base/Ray3DIntersectGeometry3DElementsBase.h"

#include <algorithm>
#include <memory>
#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <limits>
#include <chrono>
#include <fstream>

namespace Ray3DIntersectGeometry3DElementsAabbBvhTree3Std {

	// 精度
	double kAABBAccuracy = 0.01;

	// AABB盒
	struct AABB {
		// AABB盒表面积
		double Surface;
		Point3D Min;
		Point3D Max;
		// AABB盒几何元素元索引
		std::unordered_set<int> PointIndex;
	};


	// BvhNode
	struct AabbBvhTreeNode {

		// 该节点是否为叶子节点
		bool IsLeaf;

		double Radius;
		// 该节点的AABB盒
		Point3D Center;
		// 左叶子节点
		std::unique_ptr<AabbBvhTreeNode> Left;
		// 右叶子节点
		std::unique_ptr<AabbBvhTreeNode> Right;
		// 叶子节点存储的三角形索引
		std::set<int> TriangleIndices;
		std::set<int> CornerIndices;
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
		const std::vector<Point3D>& aabbs,
		const std::vector<int>& indices,
		AABB& aabb) {

		if (indices.size() < 1 || aabbs.size() < 1) {
			return false;
		}

		aabb.Min = aabbs[indices[0]];
		aabb.Max = aabbs[indices[0]];
		aabb.PointIndex.insert(indices[0]);

		for (int i = 1; i < indices.size(); ++i) {
			aabb.PointIndex.insert(indices[i]);

			aabb.Min.x = std::min(aabb.Min.x, aabbs[indices[i]].x);
			aabb.Min.y = std::min(aabb.Min.y, aabbs[indices[i]].y);
			aabb.Min.z = std::min(aabb.Min.z, aabbs[indices[i]].z);

			aabb.Max.x = std::max(aabb.Max.x, aabbs[indices[i]].x);
			aabb.Max.y = std::max(aabb.Max.y, aabbs[indices[i]].y);
			aabb.Max.z = std::max(aabb.Max.z, aabbs[indices[i]].z);
		}

		aabb.Min.x = aabb.Min.x - kAABBAccuracy;
		aabb.Min.y = aabb.Min.y - kAABBAccuracy;
		aabb.Min.z = aabb.Min.z - kAABBAccuracy;

		aabb.Max.x = aabb.Max.x + kAABBAccuracy;
		aabb.Max.y = aabb.Max.y + kAABBAccuracy;
		aabb.Max.z = aabb.Max.z + kAABBAccuracy;

		aabb.Surface = CalculateAABBSurfaceArea(aabb);

		return true;

	}


	AABB GetAABBByPoint(
		const Point3D& point,
		int index) {


		AABB aabb;
		aabb.Min = point;
		aabb.Max = point;
		aabb.PointIndex.insert(index);

		aabb.Min.x = aabb.Min.x - kAABBAccuracy;
		aabb.Min.y = aabb.Min.y - kAABBAccuracy;
		aabb.Min.z = aabb.Min.z - kAABBAccuracy;

		aabb.Max.x = aabb.Max.x + kAABBAccuracy;
		aabb.Max.y = aabb.Max.y + kAABBAccuracy;
		aabb.Max.z = aabb.Max.z + kAABBAccuracy;

		aabb.Surface = CalculateAABBSurfaceArea(aabb);

		return aabb;

	}

	// 合并两个AABB
	AABB UnionAABB(const AABB& a, const AABB& b) {

		AABB res;
		res.Min.x = std::min(a.Min.x, b.Min.x);
		res.Min.y = std::min(a.Min.y, b.Min.y);
		res.Min.z = std::min(a.Min.z, b.Min.z);

		res.Max.x = std::max(a.Max.x, b.Max.x);
		res.Max.y = std::max(a.Max.y, b.Max.y);
		res.Max.z = std::max(a.Max.z, b.Max.z);

		res.Surface = CalculateAABBSurfaceArea(res);

		for (auto index : a.PointIndex) {
			res.PointIndex.insert(index);
		}

		for (auto index : b.PointIndex) {
			res.PointIndex.insert(index);
		}

		return res;

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
			newSahSplit.LeftBox.PointIndex.clear();
			for (auto index : old.LeftBox.PointIndex) {
				newSahSplit.LeftBox.PointIndex.insert(index);
			}
		}
		{
			newSahSplit.RightBox.Surface = old.RightBox.Surface;
			newSahSplit.RightBox.Min = old.RightBox.Min;
			newSahSplit.RightBox.Max = old.RightBox.Max;
			newSahSplit.RightBox.PointIndex.clear();
			for (auto index : old.RightBox.PointIndex) {
				newSahSplit.RightBox.PointIndex.insert(index);
			}
		}
	}


	bool FindValue(const std::unordered_set<int>& mySet, int value) {

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
		newAABB.PointIndex.clear();
		for (auto index : old.PointIndex) {
			newAABB.PointIndex.insert(index);
		}

	}


	SahSplit CalculateSahSplit(
		const std::vector<Point3D>& points,
		const std::vector<int>& sortedIndices,
		int right_start_index) {

		SahSplit split;

		AABB leftBox;
		leftBox.Min = points[sortedIndices[0]];
		leftBox.Max = points[sortedIndices[0]];

		split.LeftBoxesIndex.push_back(sortedIndices[0]);
		for (int i = 1; i < right_start_index; ++i) {
			split.LeftBoxesIndex.push_back(sortedIndices[i]);

			leftBox.Min.x = std::min(leftBox.Min.x, points[sortedIndices[i]].x);
			leftBox.Min.y = std::min(leftBox.Min.y, points[sortedIndices[i]].y);
			leftBox.Min.z = std::min(leftBox.Min.z, points[sortedIndices[i]].z);
			leftBox.Max.x = std::max(leftBox.Max.x, points[sortedIndices[i]].x);
			leftBox.Max.y = std::max(leftBox.Max.y, points[sortedIndices[i]].y);
			leftBox.Max.z = std::max(leftBox.Max.z, points[sortedIndices[i]].z);
		}


		leftBox.Min.x = leftBox.Min.x - kAABBAccuracy;
		leftBox.Min.y = leftBox.Min.y - kAABBAccuracy;
		leftBox.Min.z = leftBox.Min.z - kAABBAccuracy;
		leftBox.Max.x = leftBox.Max.x + kAABBAccuracy;
		leftBox.Max.y = leftBox.Max.y + kAABBAccuracy;
		leftBox.Max.z = leftBox.Max.z + kAABBAccuracy;

		leftBox.Surface = right_start_index * CalculateAABBSurfaceArea(leftBox);

		AABB rightBox;
		rightBox.Min = points[sortedIndices[right_start_index]];
		rightBox.Max = points[sortedIndices[right_start_index]];

		split.RightBoxesIndex.push_back(sortedIndices[right_start_index]);

		for (int i = right_start_index + 1; i < sortedIndices.size(); ++i) {
			split.RightBoxesIndex.push_back(sortedIndices[i]);
			rightBox.Min.x = std::min(rightBox.Min.x, points[sortedIndices[i]].x);
			rightBox.Min.y = std::min(rightBox.Min.y, points[sortedIndices[i]].y);
			rightBox.Min.z = std::min(rightBox.Min.z, points[sortedIndices[i]].z);
			rightBox.Max.x = std::max(rightBox.Max.x, points[sortedIndices[i]].x);
			rightBox.Max.y = std::max(rightBox.Max.y, points[sortedIndices[i]].y);
			rightBox.Max.z = std::max(rightBox.Max.z, points[sortedIndices[i]].z);
		}

		rightBox.Min.x = rightBox.Min.x - kAABBAccuracy;
		rightBox.Min.y = rightBox.Min.y - kAABBAccuracy;
		rightBox.Min.z = rightBox.Min.z - kAABBAccuracy;
		rightBox.Max.x = rightBox.Max.x + kAABBAccuracy;
		rightBox.Max.y = rightBox.Max.y + kAABBAccuracy;
		rightBox.Max.z = rightBox.Max.z + kAABBAccuracy;

		rightBox.Surface = (sortedIndices.size() - right_start_index) * CalculateAABBSurfaceArea(rightBox);

		split.BestCost = leftBox.Surface + rightBox.Surface;
		CopyAABB(leftBox, split.LeftBox);
		CopyAABB(rightBox, split.RightBox);

		return split;

	}

	SahSplit FindBestSplit(
		const std::vector<Point3D>& points,
		std::vector<int>& sortedIndices)
	{

		const int SAH_STEP = 1;

		SahSplit bestSplit;
		bestSplit.BestCost = std::numeric_limits<double>::max();

		if (points.size() < 1 || sortedIndices.size() < 1) {
			return bestSplit;
		}
		else if (sortedIndices.size() == 1) {

			AABB leftBox = GetAABBByPoint(points[sortedIndices[0]], sortedIndices[0]);

			bestSplit.BestCost = leftBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.PointIndex.clear();
				for (auto index : leftBox.PointIndex) {
					bestSplit.LeftBox.PointIndex.insert(index);
				}

			}
			return bestSplit;
		}
		else if (sortedIndices.size() == 2) {
			AABB leftBox = GetAABBByPoint(points[sortedIndices[0]], sortedIndices[0]);
			AABB rightBox = GetAABBByPoint(points[sortedIndices[1]], sortedIndices[1]);
			bestSplit.BestCost = leftBox.Surface + rightBox.Surface;
			{
				bestSplit.LeftBoxesIndex.push_back(sortedIndices[0]);
				bestSplit.LeftBox.Min = leftBox.Min;
				bestSplit.LeftBox.Max = leftBox.Max;
				bestSplit.LeftBox.Surface = leftBox.Surface;
				bestSplit.LeftBox.PointIndex.clear();
				for (auto index : leftBox.PointIndex) {
					bestSplit.LeftBox.PointIndex.insert(index);
				}

			}
			{
				bestSplit.RightBoxesIndex.push_back(sortedIndices[1]);
				bestSplit.RightBox.Min = rightBox.Min;
				bestSplit.RightBox.Max = rightBox.Max;
				bestSplit.RightBox.Surface = rightBox.Surface;
				bestSplit.RightBox.PointIndex.clear();
				for (auto index : rightBox.PointIndex) {
					bestSplit.RightBox.PointIndex.insert(index);
				}

			}
			return bestSplit;
		}


		for (int axis = 0; axis < 3; ++axis) {
			// 按当前轴对索引排序
			std::sort(sortedIndices.begin(), sortedIndices.end(),
				[&](int a, int b) { return GetAxis(points[a], axis) < GetAxis(points[b], axis); });

			// 遍历候选分割点
			for (size_t i = 1; i < sortedIndices.size() - 1; i += SAH_STEP) {

				int right_start_index = (int)i;
				SahSplit split = CalculateSahSplit(points, sortedIndices, right_start_index);

				if (split.BestCost < bestSplit.BestCost) {
					CopySahSplit(split, bestSplit);
				}
			}
		}
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
	std::unique_ptr<AabbBvhTreeNode> BuildAabbBvhTreeRecursive(
		int depth,
		int maxLeafSize,
		int maxDepth,
		const std::vector<Point3D>& points,
		std::vector<int>& indices) {

		auto root_node = std::make_unique<AabbBvhTreeNode>();
		root_node->IsLeaf = false;
		if (!GetAABBByPoints(points, indices, root_node->Bound)) {
			return std::unique_ptr<AabbBvhTreeNode>();
		}

		// 终止条件：叶子节点
		if (indices.size() <= maxLeafSize || depth >= maxDepth) {
			root_node->IsLeaf = true;
			return root_node;
		}


		// 寻找最优分割
		SahSplit split = FindBestSplit(points, indices);


		// 递归构建子树
		if (!split.LeftBoxesIndex.empty()) {
			root_node->Left = BuildAabbBvhTreeRecursive(depth + 1, maxLeafSize, maxDepth, points, split.LeftBoxesIndex);
		}

		if (!split.RightBoxesIndex.empty()) {
			root_node->Right = BuildAabbBvhTreeRecursive(depth + 1, maxLeafSize, maxDepth, points, split.RightBoxesIndex);
		}

		return root_node;
	}
}

namespace Ray3DIntersectGeometry3DElementsAabbBvhTree3Std {

	std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustTriangleAccelerateStruct> globalExhaustTriangleAccelerateStructs;



	std::unique_ptr<AabbBvhTreeNode> BuildPoint3DAabbBvhTreeNode(
		int maxLeafSize,
		int maxDepth,
		const Scenario3D& scenario) {
		// 准备数据

		std::vector<Point3D> points(scenario.pointsCount);
		for (int i = 0; i < scenario.pointsCount; ++i) {
			points[i] = scenario.scenario_point3d_set[i];
		}

		// 创建初始索引列表
		std::vector<int> indices(points.size());
		for (int i = 0; i < points.size(); ++i) {
			indices[i] = i;
		}

		// 递归构建Tri-BVH-Tree
		return BuildAabbBvhTreeRecursive(0, maxLeafSize, maxDepth, points, indices);
	}


	bool Triangle3DInBox(
		const Triangle3D& triangle, const AABB& aabb) {


		bool state1 = FindValue(aabb.PointIndex, triangle.triangleP1Index);
		bool state2 = FindValue(aabb.PointIndex, triangle.triangleP2Index);
		bool state3 = FindValue(aabb.PointIndex, triangle.triangleP3Index);

		if (state1 && state2 && state3) {
			return true;
		}
		else {
			return false;
		}

	}

	void AddTriangleToAabbBvhTreeNode(
		int triangleIndex,
		const Triangle3D& triangle,
		std::unique_ptr<AabbBvhTreeNode>& root) {

		bool state1 = Triangle3DInBox(triangle, root->Bound);
		bool state2 = false;
		bool state3 = false;

		if (root->Left) {
			state2 = Triangle3DInBox(triangle, root->Left->Bound);
		}
		if (root->Right) {
			state3 = Triangle3DInBox(triangle, root->Right->Bound);
		}

		if (!state1) {
			std::cout << "Triangle out of AABB" << std::endl;
			return;
		}
		if (!state2 && !state3) {
			root->TriangleIndices.insert(triangleIndex);
			return;
		}
		if (state2) {
			AddTriangleToAabbBvhTreeNode(triangleIndex, triangle, root->Left);
		}

		if (state3) {
			AddTriangleToAabbBvhTreeNode(triangleIndex, triangle, root->Right);
		}


	}

	void BuildTriangleAabbBvhTreeNode(
		const Scenario3D& scenario,
		std::unique_ptr<AabbBvhTreeNode>& root) {


		for (int i = 0; i < scenario.trianglesCount; ++i) {
			const Triangle3D& triangle = scenario.scenario_triangle3d_set[i];

			AddTriangleToAabbBvhTreeNode(i, triangle, root);

		}

	}

}

namespace Ray3DIntersectGeometry3DElementsAabbBvhTree3Std {

	std::vector<Ray3DIntersectGeometry3DElementsBaseStd::ExhaustCornerAccelerateStruct> globalExhaustCornerAccelerateStructs;


	bool Corner3DInBox(
		const Corner3D& corner,
		const AABB& aabb) {


		bool state1 = FindValue(aabb.PointIndex, corner.p1Index);
		bool state2 = FindValue(aabb.PointIndex, corner.p2Index);

		if (state1 && state2) {
			return true;
		}
		else {
			return false;
		}

	}

	void AddCornerToAabbBvhTreeNode(
		int triangleIndex,
		const Corner3D& corner,
		std::unique_ptr<AabbBvhTreeNode>& root) {

		bool state1 = Corner3DInBox(corner, root->Bound);
		bool state2 = false;
		bool state3 = false;

		if (root->Left) {
			state2 = Corner3DInBox(corner, root->Left->Bound);
		}
		if (root->Right) {
			state3 = Corner3DInBox(corner, root->Right->Bound);
		}

		if (!state1) {
			std::cout << "Triangle out of AABB" << std::endl;
			return;
		}
		if (!state2 && !state3) {
			root->TriangleIndices.insert(triangleIndex);
			return;
		}
		if (state2) {
			AddCornerToAabbBvhTreeNode(triangleIndex, corner, root->Left);
		}

		if (state3) {
			AddCornerToAabbBvhTreeNode(triangleIndex, corner, root->Right);
		}


	}

	void BuildCornerAabbBvhTreeNode(
		const Scenario3D& scenario,
		std::unique_ptr<AabbBvhTreeNode>& root) {

		for (int i = 0; i < scenario.cornersCount; ++i) {
			const Corner3D& corner = scenario.scenario_corner3d_set[i];

			AddCornerToAabbBvhTreeNode(i, corner, root);

		}
	}
}



namespace Ray3DIntersectGeometry3DElementsAabbBvhTree3Std {


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

}

namespace Ray3DIntersectGeometry3DElementsAabbBvhTree3Std {

	void ToString(const std::unique_ptr<AabbBvhTreeNode>& root, int depth, std::ostringstream& oss) {

		if (root->Bound.PointIndex.size() > 0) {

			oss << depth << "," << root->Bound.Min.x << "," << root->Bound.Min.y << "," << root->Bound.Min.z
				<< "," << root->Bound.Max.x << "," << root->Bound.Max.y << "," << root->Bound.Max.z
				<< std::endl;

		}

		if (root->Left) {
			ToString(root->Left, depth + 1, oss);
		}
		if (root->Right) {
			ToString(root->Right, depth + 1, oss);
		}
	}

	void CalculateAabbBvhTreeNodeCenterAndRadius(std::unique_ptr<AabbBvhTreeNode>&root) {
		if (!root) {
			return;
		}

		double centerX = (root->Bound.Min.x + root->Bound.Max.x) *0.5;
		double centerY = (root->Bound.Min.y + root->Bound.Max.y) *0.5;
		double centerZ = (root->Bound.Min.z + root->Bound.Max.z) *0.5;

		double dx = root->Bound.Max.x - root->Bound.Min.x;
		double dy = root->Bound.Max.y - root->Bound.Min.y;
		double dz = root->Bound.Max.z - root->Bound.Min.z;

		double radius = std::sqrt(dx*dx + dy*dy + dz*dz) * 0.5;

		root->Center.x = centerX;
		root->Center.y = centerY;
		root->Center.z = centerZ;

		root->Radius = radius;

		CalculateAabbBvhTreeNodeCenterAndRadius(root->Left);
		CalculateAabbBvhTreeNodeCenterAndRadius(root->Right);

	}

	std::unique_ptr<AabbBvhTreeNode> RootAabbBvhTree;



	void InitializeAabbBvhTree(const Scenario3D& scenario, bool switchCorner, double cornerRadius) {

		Ray3DIntersectGeometry3DElementsBaseStd::InitExhaustTriangleAccelerateStruct(scenario, globalExhaustTriangleAccelerateStructs);

		if (switchCorner) {

			kAABBAccuracy = 2.0 * cornerRadius;

			Ray3DIntersectGeometry3DElementsBaseStd::InitExhaustCornerAccelerateStruct(scenario, globalExhaustCornerAccelerateStructs);
		}


		RootAabbBvhTree = std::move(BuildPoint3DAabbBvhTreeNode(2, 7, scenario));

		//std::ostringstream oss;
		//oss << std::fixed << std::setprecision(15);
		//ToString(RootAabbBvhTree, 0, oss);
		////std::cout << oss.str();
		////输出到txt文件
		//std::ofstream ofs("AabbBvhTree.txt", std::ios::out);
		//ofs << oss.str();
		//ofs.close();

		BuildTriangleAabbBvhTreeNode(scenario, RootAabbBvhTree);
		//计算每个节点的中心点以及包围球半径
		CalculateAabbBvhTreeNodeCenterAndRadius(RootAabbBvhTree);

		if (switchCorner) {

			BuildCornerAabbBvhTreeNode(scenario, RootAabbBvhTree);

		}



	}


	void Ray3DIntersectAabbBvhTreeTriangle3D(
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
		for (auto triangleIndex : root->TriangleIndices) {
			triangleIndices.insert(triangleIndex);
		}

		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, root->Left, triangleIndices);
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, root->Right, triangleIndices);

	}


	void CalculateRay3DIntersectTriangle3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		Ray3DIntersectGeometry3DElementResult& result) {

		std::set<int> triangleIndices;
		Ray3DIntersectAabbBvhTreeTriangle3D(ray_origin, ray_direction, RootAabbBvhTree, triangleIndices);

		Ray3DIntersectGeometry3DElementsBaseStd::CalculateRay3DIntersectTriangle3DExhaustFirstAccelerate(ray_origin, ray_direction, globalExhaustTriangleAccelerateStructs, triangleIndices, result);
	}


	void Ray3DIntersectAabbBvhTreeCorner3D(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		const std::unique_ptr<AabbBvhTreeNode>& root,
		std::set<int>& cornerIndices) {

		if (!root) {
			return;
		}

		bool isIntersect = Ray3DIntersectBall(ray_origin, ray_direction, root->Center, root->Radius);
		if (!isIntersect) {
			return;
		}
		for (auto cornerIndex : root->CornerIndices) {
			cornerIndices.insert(cornerIndex);
		}

		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, root->Left, cornerIndices);
		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, root->Right, cornerIndices);

	}

	void CalculateRay3DIntersectCorner3DAabbBvhTreeFirst(
		const Point3D& ray_origin,
		const Point3D& ray_direction,
		double cornerRadius,
		Ray3DIntersectGeometry3DElementResult& result) {

		std::set<int> cornerIndices;

		Ray3DIntersectAabbBvhTreeCorner3D(ray_origin, ray_direction, RootAabbBvhTree, cornerIndices);

		Ray3DIntersectGeometry3DElementsBaseStd::CalculateRay3DIntersectCorner3DExhaustFirstAccelerate(ray_origin, ray_direction, cornerRadius, globalExhaustCornerAccelerateStructs, cornerIndices, result);

	}

}



