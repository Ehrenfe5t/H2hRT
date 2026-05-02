

#include"HdQBuildGeometryPathDTest.h"
#include"HdQBuildGeometryPathD.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include<iostream>

namespace BuildGeometryPathDTestStd {

	//18-20s
	void Demo01() {

		int numbersOfGuess = 5;
		int maxLevel = 11;
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		for (int i = 0; i < 2000e4; i = i + 1) {
			BuildGeometryPathDStd::GeometryPathDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.cornerIndex = i;
			geometryPathDInputParameter.tx_location.x = -10;
			geometryPathDInputParameter.tx_location.y = i / 800000.0;
			geometryPathDInputParameter.tx_location.z = 5;
			geometryPathDInputParameter.rx_location.x = 20;
			geometryPathDInputParameter.rx_location.y = i / 800000.0;
			geometryPathDInputParameter.rx_location.z = 5;
			geometryPathDInputParameter.segment1.start.x = -1;
			geometryPathDInputParameter.segment1.start.y = 0;
			geometryPathDInputParameter.segment1.start.z = 0;
			geometryPathDInputParameter.segment1.end.x = 11;
			geometryPathDInputParameter.segment1.end.y = 0;
			geometryPathDInputParameter.segment1.end.z = 0;
			BuildGeometryPathDStd::BuildGeometryPathD(numbersOfGuess, maxLevel, geometryPathDInputParameter, paths);
			auto nodeBase = paths[paths.size() - 1][0];
			GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* node = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)nodeBase;

			//std::cout<<i<<"\t"<< node->cornerIndex << std::endl;
		}

		std::cout << paths.size() << std::endl;

		GeometryRTMultiPathBaseNodeStd::Free(paths);
		

	}

	void Demo02() {

		int numbersOfGuess = 3;
		int maxLevel = 9;
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		std::vector<BuildGeometryPathDStd::GeometryPathDInputParameter> geometryPathDInputParameters;
		for (int i = 0; i < 2000e4; i = i + 1) {
			BuildGeometryPathDStd::GeometryPathDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.cornerIndex = i;
			geometryPathDInputParameter.tx_location.x = -10;
			geometryPathDInputParameter.tx_location.y = i / 800000.0;
			geometryPathDInputParameter.tx_location.z = 5;
			geometryPathDInputParameter.rx_location.x = 20;
			geometryPathDInputParameter.rx_location.y = i / 800000.0;
			geometryPathDInputParameter.rx_location.z = 5;
			geometryPathDInputParameter.segment1.start.x = -1;
			geometryPathDInputParameter.segment1.start.y = 0;
			geometryPathDInputParameter.segment1.start.z = 0;
			geometryPathDInputParameter.segment1.end.x = 11;
			geometryPathDInputParameter.segment1.end.y = 0;
			geometryPathDInputParameter.segment1.end.z = 0;
			geometryPathDInputParameters.emplace_back(geometryPathDInputParameter);

		}
		BuildGeometryPathDStd::BuildGeometryPathDs(numbersOfGuess, maxLevel, geometryPathDInputParameters, paths);
		auto nodeBase = paths[paths.size() - 1][0];
		GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* node = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)nodeBase;
		GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode cur;
		std::cout << (int)cur.GetPropagationType() << std::endl;
		std::cout << (int)node->GetPropagationType() << std::endl;
		std::cout << paths.size() << std::endl;
		GeometryRTMultiPathBaseNodeStd::Free(paths);
	}

	void Demo03() {

		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		std::vector<BuildGeometryPathDStd::GeometryPathDInputParameter> geometryPathDInputParameters;
		for (int i = 0; i < 1000e4; i = i + 1) {
			BuildGeometryPathDStd::GeometryPathDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.cornerIndex = i;
			geometryPathDInputParameter.tx_location.x = -10;
			geometryPathDInputParameter.tx_location.y = i / 800000.0;
			geometryPathDInputParameter.tx_location.z = 5;
			geometryPathDInputParameter.rx_location.x = 20;
			geometryPathDInputParameter.rx_location.y = i / 800000.0;
			geometryPathDInputParameter.rx_location.z = 5;
			geometryPathDInputParameter.segment1.start.x = -1;
			geometryPathDInputParameter.segment1.start.y = 0;
			geometryPathDInputParameter.segment1.start.z = 0;
			geometryPathDInputParameter.segment1.end.x = 11;
			geometryPathDInputParameter.segment1.end.y = 0;
			geometryPathDInputParameter.segment1.end.z = 0;
			geometryPathDInputParameters.emplace_back(geometryPathDInputParameter);

		}
		BuildGeometryPathDStd::BuildGeometryPathDsByAnalyticalSolution(geometryPathDInputParameters, paths);
		//auto nodeBase = paths[paths.size() - 1][0];
		//GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* node = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)nodeBase;
		GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode cur;
		//std::cout << (int)cur.GetPropagationType() << std::endl;
		//std::cout << (int)node->GetPropagationType() << std::endl;
		std::cout << paths.size() << std::endl;
		GeometryRTMultiPathBaseNodeStd::Free(paths);
	}
}