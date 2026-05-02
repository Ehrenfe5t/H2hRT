#pragma once

#include"QzQGeometryRTMultiPathBaseNode.h"
namespace GeometryRTMultiPathRNodeStd {

	class GeometryRTMultiPathRNode:public GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode
	{
	public:
		int triangleIndex;
		GeometryRTMultiPathRNode();
		GeometryRTMultiPathRNode(int triangleIndex, const Point3DStd::Point3D& point);
		~GeometryRTMultiPathRNode();
		PropagationTypeStd::PropagationType GetPropagationType() const override;
	private:

	};


}