#pragma once

#include"QzQGeometryRTMultiPathBaseNode.h"

namespace GeometryRTMultiPathDNodeStd {

	class GeometryRTMultiPathDNode:public GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode
	{
	public:
		int cornerIndex;
		GeometryRTMultiPathDNode();
		GeometryRTMultiPathDNode(int cornerIndex, const Point3DStd::Point3D& point);
		~GeometryRTMultiPathDNode();
		PropagationTypeStd::PropagationType GetPropagationType() const override;

	private:

	};


}