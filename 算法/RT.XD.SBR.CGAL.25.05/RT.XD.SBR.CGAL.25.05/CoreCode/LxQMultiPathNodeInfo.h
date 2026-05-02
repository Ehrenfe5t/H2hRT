#pragma once

#include"LxQPoint3D.h"
#include"LxQPropagationType.h"

namespace MultiPathNodeInfoStd {

	class MultiPathNodeInfo {
	public:
		/// <summary>
		/// ∏Ω ÙŒÔid
		/// </summary>
		int attachmentId;
		PropagationTypeStd::PropagationType type;
		Point3DStd::Point3D location;
		MultiPathNodeInfo(PropagationTypeStd::PropagationType type,const Point3DStd::Point3D& location);
		MultiPathNodeInfo();
		~MultiPathNodeInfo();
	};

}