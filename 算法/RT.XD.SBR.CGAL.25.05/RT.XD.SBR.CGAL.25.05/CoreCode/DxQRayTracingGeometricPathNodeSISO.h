#pragma once

#include"DxQRayTracingGeometricPathNodeSISOOnePath.h"

#include<mutex>
namespace RayTracingGeometricPathNodeSISOStd {


	class RayTracingGeometricPathNodeSISO
	{
	public:
		int receiverAntennaId;
		std::vector<std::vector<RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath>> paths;
		RayTracingGeometricPathNodeSISO();
		RayTracingGeometricPathNodeSISO(const RayTracingGeometricPathNodeSISO& obj);
		~RayTracingGeometricPathNodeSISO();

		void Add(const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& path);
	private:

		std::mutex mtx;
	};

}