
#include"DxQRay3DCorner3DIntersectResult.h"

#include"QzQGlobalConstant.h"
namespace Ray3DCorner3DIntersectResultStd {


	Ray3DCorner3DIntersectResult::Ray3DCorner3DIntersectResult()
	{
		this->index = -1;
		this->distance = GlobalConstantStd::BoundingBoxLength;
	}

	Ray3DCorner3DIntersectResult::~Ray3DCorner3DIntersectResult()
	{
	}

}