#include "DxQRayTracingGeometricPathNodeSISO.h"



namespace RayTracingGeometricPathNodeSISOStd {


	RayTracingGeometricPathNodeSISO::RayTracingGeometricPathNodeSISO()
	{
		this->receiverAntennaId = -1;
	}

	RayTracingGeometricPathNodeSISO::RayTracingGeometricPathNodeSISO(const RayTracingGeometricPathNodeSISO& obj)
	{
		this->receiverAntennaId = obj.receiverAntennaId;
		this->paths = obj.paths;
	}

	RayTracingGeometricPathNodeSISO::~RayTracingGeometricPathNodeSISO()
	{
	}

	void RayTracingGeometricPathNodeSISO::Add(const std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode>& path)
	{
		if (path.size() > 1) {
			RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath rayTracingGeometricPathNodeSISOOnePath;
			rayTracingGeometricPathNodeSISOOnePath.path = path;
			rayTracingGeometricPathNodeSISOOnePath.CalLength();
			mtx.lock();
			while (path.size() + 1 > this->paths.size()) {
				std::vector<RayTracingGeometricPathNodeSISOOnePathStd::RayTracingGeometricPathNodeSISOOnePath> ppp;
				this->paths.emplace_back(ppp);
			}
			this->paths[path.size()].emplace_back(rayTracingGeometricPathNodeSISOOnePath);
			mtx.unlock();
		}
	}


}