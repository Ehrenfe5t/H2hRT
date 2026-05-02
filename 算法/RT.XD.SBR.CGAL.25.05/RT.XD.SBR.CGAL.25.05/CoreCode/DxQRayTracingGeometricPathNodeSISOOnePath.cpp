
#include"DxQRayTracingGeometricPathNodeSISOOnePath.h"

#include"QzQGlobalConstant.h"
#include"QzQGeometry3DOperate.Distance.h"
namespace RayTracingGeometricPathNodeSISOOnePathStd {

	 
	

	RayTracingGeometricPathNodeSISOOnePath::RayTracingGeometricPathNodeSISOOnePath()
	{
		this->length = GlobalConstantStd::BoundingBoxLength;
	}

	RayTracingGeometricPathNodeSISOOnePath::~RayTracingGeometricPathNodeSISOOnePath()
	{
	}

	void RayTracingGeometricPathNodeSISOOnePath::CalLength()
	{
		double sum = 0.0;
		for (int i = 1;i<this->path.size();++i) {
			double distance = Geometry3DOperateStd::GetDistancePoint3DPoint3D(
				this->path[i - 1].location, this->path[i].location);
			sum += distance;
		}
		this->length = sum;
	}

}