
#include"HdQBuildGeometryPathDDTest.h"
#include"HdQBuildGeometryPathDD.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"HdQCalRunTime.h"
#include<iostream>

namespace BuildGeometryPathDDTestStd {
	void Demo01() {
		int numbersOfGuess = 5;
		int maxLevel = 11;
		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		Equation2VariableObjectStd::Equation2VariableObject equation2VariableObject;
		equation2VariableObject.x1 = 0.5;
		equation2VariableObject.x2 = 0.5;

		//(-10,10)->(0,0)->(20,20)->(30,10)
		BuildGeometryPathDDStd::GeometryPathDDInputParameter geometryPathDInputParameter;
		geometryPathDInputParameter.cornerIndex1 = 3;
		geometryPathDInputParameter.cornerIndex2 = 3 + 1;
		geometryPathDInputParameter.tx_location.x = -10;
		geometryPathDInputParameter.tx_location.y = 10;
		geometryPathDInputParameter.tx_location.z = 0;
		geometryPathDInputParameter.rx_location.x = 30;
		geometryPathDInputParameter.rx_location.y = 10;
		geometryPathDInputParameter.rx_location.z = 0;

		geometryPathDInputParameter.segment1.start.x = -1;
		geometryPathDInputParameter.segment1.start.y = 0;
		geometryPathDInputParameter.segment1.start.z = 0;
		geometryPathDInputParameter.segment1.end.x = 1;
		geometryPathDInputParameter.segment1.end.y = 0;
		geometryPathDInputParameter.segment1.end.z = 0;

		geometryPathDInputParameter.segment2.start.x = 10;
		geometryPathDInputParameter.segment2.start.y = 20;
		geometryPathDInputParameter.segment2.start.z = 0;
		geometryPathDInputParameter.segment2.end.x = 30;
		geometryPathDInputParameter.segment2.end.y = 20;
		geometryPathDInputParameter.segment2.end.z = 0;
		BuildGeometryPathDDStd::Test(equation2VariableObject, geometryPathDInputParameter);

		
		std::cout << paths.size() << std::endl;

		GeometryRTMultiPathBaseNodeStd::Free(paths);
	}

	void Demo02() {
		CalRunTimeStd::CalRunTime CalRunTime(true);
		int numbersOfGuess = 20;
		int maxLevel = 9;
		int count = 0;
		for (int i = 0; i < 1e4; i = i + 1) {
			BuildGeometryPathDDStd::GeometryPathDDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.cornerIndex1 = 3;
			geometryPathDInputParameter.cornerIndex2 = 3 + 1;
			geometryPathDInputParameter.tx_location.x = -1000;
			geometryPathDInputParameter.tx_location.y = 10;
			geometryPathDInputParameter.tx_location.z = 2*i;
			geometryPathDInputParameter.rx_location.x = 1000;
			geometryPathDInputParameter.rx_location.y = 10;
			geometryPathDInputParameter.rx_location.z = 2 * i;

			geometryPathDInputParameter.segment1.start.x = -1000;
			geometryPathDInputParameter.segment1.start.y = 0;
			geometryPathDInputParameter.segment1.start.z = 0;
			geometryPathDInputParameter.segment1.end.x = 1000;
			geometryPathDInputParameter.segment1.end.y = 0;
			geometryPathDInputParameter.segment1.end.z = 0;

			geometryPathDInputParameter.segment2.start.x = -300;
			geometryPathDInputParameter.segment2.start.y = 200;
			geometryPathDInputParameter.segment2.start.z = 0;
			geometryPathDInputParameter.segment2.end.x = 300;
			geometryPathDInputParameter.segment2.end.y = 200;
			geometryPathDInputParameter.segment2.end.z = 0;
			std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
			BuildGeometryPathDDStd::BuildGeometryPathDD(numbersOfGuess, maxLevel, geometryPathDInputParameter, paths);
			if (paths.size()>0) {
				count++;
				//std::cout << paths.size() << std::endl;
			}
			//auto nodeBase = paths[paths.size() - 1][0];
			//GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* node = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)nodeBase;

			//std::cout<<i<<"\t"<< node->cornerIndex << std::endl;
		}

		std::cout << count << std::endl;
		//GeometryRTMultiPathBaseNodeStd::Free(paths);
	}

	void Demo03() {
		CalRunTimeStd::CalRunTime CalRunTime(true);
		int numbersOfGuess = 15;
		int maxLevel = 9;
		int count = 0;
		std::vector<BuildGeometryPathDDStd::GeometryPathDDInputParameter> geometryPathDInputParameters;
		for (int i = 0; i < 1e4; i = i + 1) {
			BuildGeometryPathDDStd::GeometryPathDDInputParameter geometryPathDInputParameter;
			geometryPathDInputParameter.cornerIndex1 = 3;
			geometryPathDInputParameter.cornerIndex2 = 3 + 1;
			geometryPathDInputParameter.tx_location.x = -1000;
			geometryPathDInputParameter.tx_location.y = 10;
			geometryPathDInputParameter.tx_location.z = 2 * i;
			geometryPathDInputParameter.rx_location.x = 1000;
			geometryPathDInputParameter.rx_location.y = 10;
			geometryPathDInputParameter.rx_location.z = 2 * i;

			geometryPathDInputParameter.segment1.start.x = -1000;
			geometryPathDInputParameter.segment1.start.y = 0;
			geometryPathDInputParameter.segment1.start.z = 0;
			geometryPathDInputParameter.segment1.end.x = 1000;
			geometryPathDInputParameter.segment1.end.y = 0;
			geometryPathDInputParameter.segment1.end.z = 0;

			geometryPathDInputParameter.segment2.start.x = -300;
			geometryPathDInputParameter.segment2.start.y = 200;
			geometryPathDInputParameter.segment2.start.z = 0;
			geometryPathDInputParameter.segment2.end.x = 300;
			geometryPathDInputParameter.segment2.end.y = 200;
			geometryPathDInputParameter.segment2.end.z = 0;

			geometryPathDInputParameters.emplace_back(geometryPathDInputParameter);
			//std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
			//BuildGeometryPathDDStd::BuildGeometryPathDD(numbersOfGuess, maxLevel, geometryPathDInputParameter, paths);
			//if (paths.size() > 0) {
			//	count++;
			//	//std::cout << paths.size() << std::endl;
			//}
			//auto nodeBase = paths[paths.size() - 1][0];
			//GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode* node = (GeometryRTMultiPathDNodeStd::GeometryRTMultiPathDNode*)nodeBase;

			//std::cout<<i<<"\t"<< node->cornerIndex << std::endl;
		}

		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		BuildGeometryPathDDStd::BuildGeometryPathDDsAndParameter(numbersOfGuess, maxLevel, geometryPathDInputParameters, paths);
		std::cout << paths.size() << std::endl;
		//std::cout << count << std::endl;

		//GeometryRTMultiPathBaseNodeStd::Free(paths);
	}
}