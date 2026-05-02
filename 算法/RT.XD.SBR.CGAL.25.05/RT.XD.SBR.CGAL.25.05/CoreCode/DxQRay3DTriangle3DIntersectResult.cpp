

#include"DxQRay3DTriangle3DIntersectResult.h"
#include"QzQGlobalConstant.h"

namespace Ray3DTriangle3DIntersectResultStd {


	Ray3DTriangle3DIntersectResult::Ray3DTriangle3DIntersectResult()
	{
		this->index = -1;
		this->distance = GlobalConstantStd::BoundingBoxLength;
	}

	Ray3DTriangle3DIntersectResult::~Ray3DTriangle3DIntersectResult()
	{
	}

}