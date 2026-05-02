
#include"DxQScenarioDiffuseScatteringFace.h"


namespace ScenarioDiffuseScatteringFaceStd {

	ScenarioDiffuseScatteringFace::ScenarioDiffuseScatteringFace() {
		this->triangleIndex = -1;
		this->area = 0.0;
	}


	ScenarioDiffuseScatteringFace::ScenarioDiffuseScatteringFace(int triangleIndex, double area, const Point3DStd::Point3D& center) {
		this->triangleIndex = triangleIndex;
		this->area = area;
		this->center = center;
	}

	ScenarioDiffuseScatteringFace::~ScenarioDiffuseScatteringFace() {

	}
}