#pragma once

#include"QzQGeometryRTMultiPathBaseNode.h"

namespace GeometryRTMultiPathRxNodeStd {

	class GeometryRTMultiPathRxNode :public GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode
	{
	public:
		int id;
		GeometryRTMultiPathRxNode();
		~GeometryRTMultiPathRxNode();
		GeometryRTMultiPathRxNode(int id,const Point3DStd::Point3D& point);
		PropagationTypeStd::PropagationType GetPropagationType() const override;

	private:

	};

}