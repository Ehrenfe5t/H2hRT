


#include"QzQDRToMultiPathNodeInfo.h"

#include"HdQBuildGeometryPathDR.h"
#include"HdQBaseToMultiPathNodeInfo.h"
#include"LxQProjectDependencies.h"

namespace DRToMultiPathNodeInfoStd {


	void FindDRToMultiPathNodeInfo(
		int id_tx,
		int id_rx,
		int numbersOfGuess,
		int maxLevel,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res) {


		std::vector<BuildGeometryPathDRStd::GeometryPathDRInputParameter> geometryPathDRInputParameters;
		for (int i = 0; i < scenarioDataInformation.scenarioObject.scenarioCorner3DIndex.size(); ++i) {
			
			auto scenarioCorner3DIndex1 = scenarioDataInformation.scenarioObject.scenarioCorner3DIndex[i];
			for (int j = 0; j < scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex.size(); ++j) {
				
				auto scenarioTriangle3DIndex1 = scenarioDataInformation.scenarioObject.scenarioTriangle3DIndex[j];

				BuildGeometryPathDRStd::GeometryPathDRInputParameter geometryPathDRInputParameter;
				geometryPathDRInputParameter.cornerIndex = i;
				geometryPathDRInputParameter.triangleIndex = j;

				geometryPathDRInputParameter.tx_location = tx_location;
				geometryPathDRInputParameter.rx_location = rx_location;
				geometryPathDRInputParameter.id_tx = id_tx;
				geometryPathDRInputParameter.id_rx = id_rx;


				geometryPathDRInputParameter.segment1.start =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P1Index];

				geometryPathDRInputParameter.segment1.end =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioCorner3DIndex1.P2Index];


				geometryPathDRInputParameter.triangle1.p1 =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioTriangle3DIndex1.TriangleP1Index];
				geometryPathDRInputParameter.triangle1.p2 =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioTriangle3DIndex1.TriangleP2Index];
				geometryPathDRInputParameter.triangle1.p3 =
					scenarioDataInformation.scenarioObject.scenarioPoint3D[scenarioTriangle3DIndex1.TriangleP3Index];
				geometryPathDRInputParameter.triangle1_n = scenarioTriangle3DIndex1.n;


				geometryPathDRInputParameters.emplace_back(geometryPathDRInputParameter);
			}
		}

		{
			std::ostringstream oss;
			oss << "组合数量：" << geometryPathDRInputParameters.size();
			ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
		}

		std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>> paths;
		BuildGeometryPathDRStd::BuildGeometryPathDRs(numbersOfGuess, maxLevel, geometryPathDRInputParameters, paths);

		//
		BaseToMultiPathNodeInfoStd::ToMultiPathNodeInfo(scenarioDataInformation, paths, res);

	}

}