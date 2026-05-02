#pragma once


#include"DxQRayTracingGeometricPathNodeSISO.h"
#include"DxQReceiverAntenna.h"
#include"DxQScenarioDataInformation.h"
#include"DxQTransmittingAntenna.h"
#include"HdQAntennaSIMOPath.h"

namespace RayTracingGeometricPathNodeSIMOStd {

	class RayTracingGeometricPathNodeSIMO
	{
	public:

		int transmittingAntennaId;
		std::vector<RayTracingGeometricPathNodeSISOStd::RayTracingGeometricPathNodeSISO> rayTracingGeometricPathNodeSISOs;

		RayTracingGeometricPathNodeSIMO(
			const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna,
			const std::vector<ReceiverAntennaStd::ReceiverAntenna>& receiverAntennas);
		~RayTracingGeometricPathNodeSIMO();

		void AddPath(int loop_index, const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& path);
	private:

	};


	void DeleteSamePath_SIMO(
		bool switchOfMultithread,
		double deduplicateRadius,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO);


	void RayTracingGeometricPathNodeToMultiPathNodeInfo_SIMO(
		bool switchOfMultithread,
		ScenarioDataInformationStd::ScenarioDataInformation& scenarioDataInformation,
		RayTracingGeometricPathNodeSIMOStd::RayTracingGeometricPathNodeSIMO& rayTracingGeometricPathNodeSIMO,
		AntennaSIMOPathStd::AntennaSIMOPath& antennaSIMOPath);

}
