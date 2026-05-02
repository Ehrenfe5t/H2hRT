#pragma once

#include"LxQPoint3D.h"
#include"LxQPropagationType.h"

namespace RayTracingGeometricPathNodeStd {

	class RayTracingGeometricPathNode
	{
	public:

		PropagationTypeStd::PropagationType type;

		/// <summary>
		/// 分别在不同的类型下表示不同的含义，碰撞的元素id
		/// </summary>
		int nodeElementId;


		Point3DStd::Point3D location;

		RayTracingGeometricPathNode();
		~RayTracingGeometricPathNode();


		RayTracingGeometricPathNode(PropagationTypeStd::PropagationType type, int nodeElementId,const Point3DStd::Point3D& location);

	private:

	};

	//需要去重
}