

#include"QzQGeometryRTMultiPathBaseNode.h"
#include"QzQGeometryRTMultiPathDNode.h"
#include"QzQGeometry3DOperate.Equals.h"

namespace GeometryRTMultiPathBaseNodeStd {



    GeometryRTMultiPathBaseNode::GeometryRTMultiPathBaseNode()
    {
    }

    PropagationTypeStd::PropagationType GeometryRTMultiPathBaseNode::GetPropagationType() const
    {
        return PropagationTypeStd::PropagationType::Null;
    }

    GeometryRTMultiPathBaseNode::~GeometryRTMultiPathBaseNode()
    {
    }

    void Free(std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& path) {
        for (auto it = path.begin(); it != path.end(); it++)
        {
            if (*it != nullptr)
            {
                delete* it;
                *it = nullptr;
            }
        }
    }

    void Free(std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths) {
        for (auto it = paths.begin(); it != paths.end(); it++)
        {
            Free(*it);
        }
    }

    bool SameNode(GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode* node1, GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode* node2) {
        return Geometry3DOperateStd::Equals_Point3D(node1->location, node2->location);
    }

    bool SamePath(std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& path1, std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>& path2) {

        if (path1.size() < 2) {
            return false;
        }
        if (path1.size()!= path2.size()) {
            return false;
        }
        for (int i = 0; i < path1.size();++i) {
            if (!SameNode(path1[i], path2[i])) {
                return false;
            }
        }
        return true;
    }

    void DeleteSamePath(std::vector<std::vector<GeometryRTMultiPathBaseNodeStd::GeometryRTMultiPathBaseNode*>>& paths)
    {
        for (int i = (int)paths.size() - 1; i > 0;--i) {
            for (int j = i - 1; j >= 0; --j) {
                if (SamePath(paths[i], paths[j])) {
                    paths.erase(paths.begin() + i);
                    break;
                }
            }
        }
    }

}