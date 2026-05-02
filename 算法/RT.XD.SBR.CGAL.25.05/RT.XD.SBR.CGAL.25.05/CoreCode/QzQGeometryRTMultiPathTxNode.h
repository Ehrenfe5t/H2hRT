#pragma once

#include"QzQGeometryRTMultiPathBaseNode.h"

namespace GeometryRTMultiPathTxNodeStd {

	class GeometryRTMultiPathTxNode :public GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode
	{
	public:
		int id;
		GeometryRTMultiPathTxNode();
		GeometryRTMultiPathTxNode(int id,const Point3DStd::Point3D& point);
		~GeometryRTMultiPathTxNode();
		PropagationTypeStd::PropagationType GetPropagationType() const override;

	private:

	};

}