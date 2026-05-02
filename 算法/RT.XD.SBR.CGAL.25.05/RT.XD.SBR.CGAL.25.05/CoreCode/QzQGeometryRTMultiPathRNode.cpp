

#include"QzQGeometryRTMultiPathRNode.h"
namespace GeometryRTMultiPathRNodeStd {


	GeometryRTMultiPathRNode::GeometryRTMultiPathRNode()
	{
		this->triangleIndex = -1;
	}

	GeometryRTMultiPathRNode::GeometryRTMultiPathRNode(int triangleIndex, const Point3DStd::Point3D& point)
	{
		this->triangleIndex = triangleIndex;
		this->location = point;
	}

	GeometryRTMultiPathRNode::~GeometryRTMultiPathRNode()
	{
	}

	PropagationTypeStd::PropagationType GeometryRTMultiPathRNode::GetPropagationType() const
	{
		return PropagationTypeStd::PropagationType::Reflection;
	}

}