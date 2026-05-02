#pragma once


#include"DxQScenarioDataInformation.h"
#include"DxQRayTracingGeometricPathNode.h"
#include"LxQPoint3D.h"

namespace DDToMultiPathNodeInfoStd {


	void FindDDToMultiPathNodeInfo(
		int id_tx,
		int id_rx,
		int numbersOfGuess,
		int maxLevel,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res);

	void FindDDToMultiPathNodeInfo_Accelerated(
		int id_tx,
		int id_rx,
		int numbersOfGuess,
		int maxLevel,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res);

}