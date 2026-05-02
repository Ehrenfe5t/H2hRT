#include "DxQRayTracingGeometricPathNode.h"


namespace RayTracingGeometricPathNodeStd {



    RayTracingGeometricPathNode::RayTracingGeometricPathNode()
    {
        this->type = PropagationTypeStd::PropagationType::Null;
        this->nodeElementId = -1;
    }

    RayTracingGeometricPathNode::~RayTracingGeometricPathNode()
    {
    }

    RayTracingGeometricPathNode::RayTracingGeometricPathNode(PropagationTypeStd::PropagationType type, 
        int nodeElementId, const Point3DStd::Point3D& location)
    {
        this->type = type;
        this->nodeElementId = nodeElementId;
        this->location = location;
    }

}