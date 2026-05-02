

#include"QzQGeometryRTMultiPathRxNode.h"

namespace GeometryRTMultiPathRxNodeStd {

	GeometryRTMultiPathRxNode::GeometryRTMultiPathRxNode()
	{
		this->id = -1;
	}

	GeometryRTMultiPathRxNode::~GeometryRTMultiPathRxNode()
	{
	}

	GeometryRTMultiPathRxNode::GeometryRTMultiPathRxNode(int id,const Point3DStd::Point3D& point)
	{
		this->id = id;
		this->location = point;
	}

	PropagationTypeStd::PropagationType GeometryRTMultiPathRxNode::GetPropagationType() const
	{
		return PropagationTypeStd::PropagationType::ReceiverAntenna;
	}

}