#pragma once

#include"LxQPoint3D.h"

namespace ScenarioDiffuseScatteringFaceStd {

	class ScenarioDiffuseScatteringFace {
	public:
		int triangleIndex;
		double area;
		Point3DStd::Point3D center;

		ScenarioDiffuseScatteringFace();

		ScenarioDiffuseScatteringFace(int triangleIndex, double area, const Point3DStd::Point3D& center);

		~ScenarioDiffuseScatteringFace();
	};
}