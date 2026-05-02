

#include"QzQGeometryRTMultiPathTxNode.h"

namespace GeometryRTMultiPathTxNodeStd {

	GeometryRTMultiPathTxNode::GeometryRTMultiPathTxNode()
	{
		this->id = -1;
	}

	GeometryRTMultiPathTxNode::~GeometryRTMultiPathTxNode()
	{
	}
	GeometryRTMultiPathTxNode::GeometryRTMultiPathTxNode(int id , const Point3DStd::Point3D& point) {
		this->id = id;
		this->location = point;
	}

	PropagationTypeStd::PropagationType GeometryRTMultiPathTxNode::GetPropagationType() const {
		return PropagationTypeStd::PropagationType::TransmittingAntenna;
	}
}