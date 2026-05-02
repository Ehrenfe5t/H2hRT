

#include"QzQGeometryRTMultiPathDNode.h"

namespace GeometryRTMultiPathDNodeStd {


	GeometryRTMultiPathDNode::GeometryRTMultiPathDNode() {
		this->cornerIndex = -1;
	}
	GeometryRTMultiPathDNode::GeometryRTMultiPathDNode(int cornerIndex, const Point3DStd::Point3D& point) {

		this->cornerIndex = cornerIndex;
		this->location = point;
	}

	GeometryRTMultiPathDNode::~GeometryRTMultiPathDNode() {

	}

	PropagationTypeStd::PropagationType GeometryRTMultiPathDNode::GetPropagationType() const
	{
		return PropagationTypeStd::PropagationType::Diffraction;
	}
}