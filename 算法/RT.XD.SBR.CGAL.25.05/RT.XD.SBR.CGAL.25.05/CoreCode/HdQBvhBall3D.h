#pragma once

#include"HdQBvhBall3DNodeInfo.h"
#include"HdQArrayInt.h"

#include<list>
#include<memory>
#include<atomic>
#include<stack>

namespace BvhBall3DStd {


	class BvhBall3D
	{
	public:
		ArrayIntStd::ArrayInt triangleIdSet_safe;
		ArrayIntStd::ArrayInt cornerIdSet_safe;
		BvhBall3D* leftNode_safe = NULL;
		BvhBall3D* rightNode_safe = NULL;

		BvhBall3DNodeInfoStd::BvhBall3DNodeInfo nodeInfo;


		/// <summary>
		/// 如果当前结点不是叶子节点，triangleIdSet存储无法被包裹在子节点中的三角面元编号,
		/// 如果当前结点是叶子节点，triangleIdSet存储包围盒内的三角面元编号,
		/// </summary>
		std::set<int> triangleIdSet;
		std::set<int> cornerIdSet;
		int GetLevel() const;
		BvhBall3D();
		BvhBall3D(int level, const BoundingBox3DStd::BoundingBox3D& box3D);
		~BvhBall3D();


		bool UpdateSafeIdSetInfo();

		bool IsTriangle3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3);
		bool IsLineSegment3DInBoxPlus(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2);
		/// <summary>
		/// 保证最外层可以放下
		/// </summary>
		/// <param name="p1"></param>
		/// <param name="p2"></param>
		/// <param name="p3"></param>
		/// <param name="id"></param>
		void AddTriangleId(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, const Point3DStd::Point3D& p3, int id);
		void AddCornerId(const Point3DStd::Point3D& p1, const Point3DStd::Point3D& p2, int id);

		bool DelOneNullNode();
		void DelNullNode();
	private:
		bool DelNullLastNode();

		bool DelNullLastNodeLeft();
		bool DelNullLastNodeRight();

		void GetStack(BvhBall3D* p, std::stack<BvhBall3D*>& stack);
	};


}