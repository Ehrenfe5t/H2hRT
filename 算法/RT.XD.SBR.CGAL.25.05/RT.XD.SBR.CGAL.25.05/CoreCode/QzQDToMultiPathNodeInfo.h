#pragma once

#include"DxQRayTracingGeometricPathNode.h"
#include"DxQScenarioDataInformation.h"

namespace DToMultiPathNodeInfoStd {


	void FindDToMultiPathNodeInfo(
		int id_tx,
		int id_rx,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& paths);

	void FindDToMultiPathNodeInfo_Accelerated(
		int id_tx,
		int id_rx,
		const Point3DStd::Point3D& tx_location,
		const Point3DStd::Point3D& rx_location,
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res);


}