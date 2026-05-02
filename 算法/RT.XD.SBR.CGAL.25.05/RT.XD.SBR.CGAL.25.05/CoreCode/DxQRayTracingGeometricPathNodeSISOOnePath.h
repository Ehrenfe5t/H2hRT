#pragma once

#include"DxQRayTracingGeometricPathNode.h"

#include<vector>
namespace RayTracingGeometricPathNodeSISOOnePathStd {

	class RayTracingGeometricPathNodeSISOOnePath
	{
	public:
		std::vector<RayTracingGeometricPathNodeStd::RayTracingGeometricPathNode> path;
		RayTracingGeometricPathNodeSISOOnePath();
		~RayTracingGeometricPathNodeSISOOnePath();

		double length;

		void CalLength();
	private:
		
	};

}