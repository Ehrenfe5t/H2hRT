


#include"HdQBuildBvhBall3D.h"
#include<algorithm>

#include<iostream>
/// <summary>
/// 构建一个bvh树结构
/// </summary>
namespace BuildBvhBall3DStd {

	void CalChiledrenBoxesByCenter(
		int curLevel,
		BoundingBox3DStd::BoundingBox3D& curBox,
		int maxLevel,
		BvhBall3DStd::BvhBall3D* bvhBall3D) {
		if (curLevel > maxLevel) {
			return;
		}

		double lenx = curBox.max.x - curBox.min.x;
		double leny = curBox.max.y - curBox.min.y;
		double lenz = curBox.max.z - curBox.min.z;
		BoundingBox3DStd::BoundingBox3D box1;
		BoundingBox3DStd::BoundingBox3D box2;
		if (lenx >= leny && lenx >= lenz) {
			double mid = 0.5 * (curBox.max.x + curBox.min.x);
			Point3DStd::Point3D box1Minp = curBox.min;
			Point3DStd::Point3D box1Maxp(mid, curBox.max.y, curBox.max.z);
			Point3DStd::Point3D box2Minp(mid, curBox.min.y, curBox.min.z);
			Point3DStd::Point3D box2Maxp = curBox.max;

			box1.min = box1Minp;
			box1.max = box1Maxp;
			box2.min = box2Minp;
			box2.max = box2Maxp;
		}
		else if (leny >= lenx && leny >= lenz) {
			double mid = 0.5 * (curBox.max.y + curBox.min.y);
			Point3DStd::Point3D box1Minp = curBox.min;
			Point3DStd::Point3D box1Maxp(curBox.max.x, mid, curBox.max.z);
			Point3DStd::Point3D box2Minp(curBox.min.x, mid, curBox.min.z);
			Point3DStd::Point3D box2Maxp = curBox.max;
			box1.min = box1Minp;
			box1.max = box1Maxp;
			box2.min = box2Minp;
			box2.max = box2Maxp;
		}
		else if (lenz >= lenx && lenz >= leny) {
			double mid = 0.5 * (curBox.max.z + curBox.min.z);
			Point3DStd::Point3D box1Minp = curBox.min;
			Point3DStd::Point3D box1Maxp(curBox.max.x, curBox.max.y, mid);
			Point3DStd::Point3D box2Minp(curBox.min.x, curBox.min.y, mid);
			Point3DStd::Point3D box2Maxp = curBox.max;
			box1.min = box1Minp;
			box1.max = box1Maxp;
			box2.min = box2Minp;
			box2.max = box2Maxp;
		}
		else {
			return;
		}
		bvhBall3D->leftNode_safe = new BvhBall3DStd::BvhBall3D(curLevel, box1);
		bvhBall3D->rightNode_safe = new BvhBall3DStd::BvhBall3D(curLevel, box2);
		CalChiledrenBoxesByCenter(curLevel + 1, box1, maxLevel, bvhBall3D->leftNode_safe);
		CalChiledrenBoxesByCenter(curLevel + 1, box2, maxLevel, bvhBall3D->rightNode_safe);
	}


	void AddTrianglesToBoxes(
		const std::vector<Point3DStd::Point3D>& scenarioPoint3D,
		const std::vector<ScenarioTriangle3DIndexStd::ScenarioTriangle3DIndex>& scenarioTriangle3DIndex,
		BvhBall3DStd::BvhBall3D& bvhBall3D) {

		for (int i = 0; i < scenarioTriangle3DIndex.size(); ++i) {
			auto p1 = scenarioPoint3D[scenarioTriangle3DIndex[i].TriangleP1Index];
			auto p2 = scenarioPoint3D[scenarioTriangle3DIndex[i].TriangleP2Index];
			auto p3 = scenarioPoint3D[scenarioTriangle3DIndex[i].TriangleP3Index];
			
			if (bvhBall3D.IsTriangle3DInBoxPlus(p1,p2,p3)) {
				bvhBall3D.AddTriangleId(p1, p2, p3, i);
			}
			else {
				std::cout << __FILE__ << "\t" << __LINE__ << std::endl;
			}
		}

	}
	void AddCornerToBoxes(
		const std::vector<Point3DStd::Point3D>& scenarioPoint3D,
		const std::vector<ScenarioCorner3DIndexStd::ScenarioCorner3DIndex>& scenarioCorner3DIndex,
		BvhBall3DStd::BvhBall3D& bvhBall3D) {

		for (int i = 0; i < scenarioCorner3DIndex.size(); ++i) {
			auto p1 = scenarioPoint3D[scenarioCorner3DIndex[i].P1Index];
			auto p2 = scenarioPoint3D[scenarioCorner3DIndex[i].P2Index];

			if (bvhBall3D.IsLineSegment3DInBoxPlus(p1, p2)) {
				bvhBall3D.AddCornerId(p1, p2, i);
			}
			else {
				std::cout << __FILE__ << "\t" << __LINE__ << std::endl;
			}
		}
	}

	//SAH算法第一步，找到自适应的切割轴是x,y,z
	/// <summary>
	/// 输入 点、三角形、边的信息构建一个bvh树
	/// </summary>
	/// <param name="bvhBall3DParameter"></param>
	/// <param name="scenarioObject"></param>
	/// <returns></returns>
	void BuildBvhBall3D(
		const BvhBall3DParameterStd::BvhBall3DParameter& bvhBall3DParameter,
		const ScenarioObjectStd::ScenarioObject& scenarioObject,
		BvhBall3DStd::BvhBall3D& bvhBall3D) {

		BoundingBox3DStd::BoundingBox3D box(scenarioObject.scenarioMinPoint3D, scenarioObject.scenarioMaxPoint3D);
		

		bvhBall3D.nodeInfo.SetLevel(1);
		bvhBall3D.nodeInfo.ReBuildByBox3D(box);
		CalChiledrenBoxesByCenter(2, box, bvhBall3DParameter.maxLevel, &bvhBall3D);

		//添加三角形进去
		AddTrianglesToBoxes(scenarioObject.scenarioPoint3D, scenarioObject.scenarioTriangle3DIndex, bvhBall3D);
		//添加边进去
		AddCornerToBoxes(scenarioObject.scenarioPoint3D, scenarioObject.scenarioCorner3DIndex, bvhBall3D);

		//删除没有元素的节点
		bvhBall3D.DelNullNode();

		bvhBall3D.UpdateSafeIdSetInfo();

	}

}