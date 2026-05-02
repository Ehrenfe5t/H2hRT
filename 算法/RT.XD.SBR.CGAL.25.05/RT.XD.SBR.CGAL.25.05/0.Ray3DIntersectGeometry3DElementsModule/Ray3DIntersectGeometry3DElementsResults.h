#pragma once

#include"0.Ray3DIntersectGeometry3DElementsModule.Input.h"

struct Ray3DIntersectGeometry3DElementResult
{
	/// <summary>
	/// 碰撞类型，0表示和三角形碰撞了，1表示和边碰撞了
	/// </summary>
	int type;
	/// <summary>
	/// 碰撞的元素编号
	/// </summary>
	int elementIndex;
	double distance;
	Point3D location;


};


struct Ray3DIntersectGeometry3DElementsResults {

	std::vector<Ray3DIntersectGeometry3DElementResult> results;

};