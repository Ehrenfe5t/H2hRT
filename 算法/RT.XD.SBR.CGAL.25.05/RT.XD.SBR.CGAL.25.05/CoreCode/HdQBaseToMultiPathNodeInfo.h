#pragma once

#include"DxQScenarioDataInformation.h"
#include"QzQGeometryRTMultiPathBaseNode.h"
#include"DxQRayTracingGeometricPathNode.h"

namespace BaseToMultiPathNodeInfoStd {

	void ToMultiPathNodeInfo(
		const ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		const std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& objs,
		std::vector<std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>*>& res);
}

