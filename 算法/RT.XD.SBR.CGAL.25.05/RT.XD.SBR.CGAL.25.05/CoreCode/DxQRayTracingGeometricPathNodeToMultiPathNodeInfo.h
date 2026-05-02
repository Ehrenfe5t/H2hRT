#pragma once


#include"DxQRayTracingGeometricPathNode.h"
#include"DxQScenarioDataInformation.h"
#include"LxQMultiPathNodeInfo.h"

namespace RayTracingGeometricPathNodeToMultiPathNodeInfoStd {


	bool RayTracingGeometricPathNodeToMultiPathNodeInfo_Path(
		const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& input,
		ScenarioDataInformationStd::ScenarioDataInformation* scenarioDataInformation,
		std::vector<MultiPathNodeInfoStd::MultiPathNodeInfo*>& res);


}