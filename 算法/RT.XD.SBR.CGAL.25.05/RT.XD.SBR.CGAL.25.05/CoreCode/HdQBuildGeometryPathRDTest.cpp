
#include"HdQBuildGeometryPathRDTest.h"

#include"HdQBuildGeometryPathRD.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometry3DOperate.NormalVector.h"
#include<iostream>
namespace BuildGeometryPathRDTestStd {


	void Demo01() {

		int numbersOfGuess = 10;
		int maxLevel = 11;
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		BuildGeometryPathDRStd::GeometryPathDRInputParameter geometryPathDRInputParameter;
		geometryPathDRInputParameter.cornerIndex = 3;
		geometryPathDRInputParameter.rx_location.x = -2;
		geometryPathDRInputParameter.rx_location.y = 2;
		geometryPathDRInputParameter.rx_location.z = 0;

		geometryPathDRInputParameter.tx_location.x = 0;
		geometryPathDRInputParameter.tx_location.y = 2;
		geometryPathDRInputParameter.tx_location.z = 0;
		geometryPathDRInputParameter.segment1.start.x = -1;
		geometryPathDRInputParameter.segment1.start.y = 0;
		geometryPathDRInputParameter.segment1.start.z = 0;
		geometryPathDRInputParameter.segment1.end.x = 1;
		geometryPathDRInputParameter.segment1.end.y = 0;
		geometryPathDRInputParameter.segment1.end.z = 0;

		geometryPathDRInputParameter.triangle1_n.x = -1;
		geometryPathDRInputParameter.triangle1_n.y = 0;
		geometryPathDRInputParameter.triangle1_n.z = 0;
		geometryPathDRInputParameter.triangleIndex = 3 + 1;

		geometryPathDRInputParameter.triangle1.p1.x = 1;
		geometryPathDRInputParameter.triangle1.p1.y = 10;
		geometryPathDRInputParameter.triangle1.p1.z = -1;

		geometryPathDRInputParameter.triangle1.p2.x = 1;
		geometryPathDRInputParameter.triangle1.p2.y = -10;
		geometryPathDRInputParameter.triangle1.p2.z = -1;

		geometryPathDRInputParameter.triangle1.p3.x = 1;
		geometryPathDRInputParameter.triangle1.p3.y = 1;
		geometryPathDRInputParameter.triangle1.p3.z = 10;
		auto N = Geometry3DOperateStd::NormalVector_Triangle_plus_unsafe(geometryPathDRInputParameter.triangle1.p1,
			geometryPathDRInputParameter.triangle1.p2,
			geometryPathDRInputParameter.triangle1.p3);

		BuildGeometryPathRDStd::BuildGeometryPathRD(numbersOfGuess, maxLevel, geometryPathDRInputParameter, paths);

		std::cout << paths.size() << std::endl;

		GeometryRTMultiPathBaseNodeStd::Free(paths);
	}


	void Demo02() {

	}


}