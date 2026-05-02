#pragma once

#include"DxQRay3DTriangle3DIntersectResult.h"
#include"DxQRay3DCorner3DIntersectResult.h"

namespace Ray3DScenario3DIntersectResultStd {


	/// <summary>
	/// //计算射线的碰撞结果
    ///(type:碰撞类型、1表示面元、2表示棱；
    ///(point:碰撞点坐标；
    ///(distance射线碰撞时射线走的长度)
    ///(index碰撞面元的编号
	/// </summary>
	class Ray3DScenario3DIntersectResult
	{
	public:
		Ray3DTriangle3DIntersectResultStd::Ray3DTriangle3DIntersectResult ray3DTriangle3DIntersectResult;
		Ray3DCorner3DIntersectResultStd::Ray3DCorner3DIntersectResult ray3DCorner3DIntersectResult;

		Ray3DScenario3DIntersectResult();
		~Ray3DScenario3DIntersectResult();

	private:

	};


}