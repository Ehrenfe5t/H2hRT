

#include"LxQMultiPathNodeInfoDiffuseScattering.h"

namespace MultiPathNodeInfoStd {
	MultiPathNodeInfoDiffuseScattering::MultiPathNodeInfoDiffuseScattering(
		int upObjectType, int downObjectType,
		int widthCoefficientOfDiffuseLobe, float roughness, double thetai, double beta, double area, double scatteringCoefficient,
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& normalVector)
	{
		this->type = PropagationTypeStd::PropagationType::DiffuseScattering;
		this->location = location;

		this->upObjectType = upObjectType;
		this->downObjectType = downObjectType;

		this->widthCoefficientOfDiffuseLobe = widthCoefficientOfDiffuseLobe;
		this->roughness = roughness;
		this->thetai = thetai;
		this->beta = beta;
		this->area = area;
		this->scatteringCoefficient = scatteringCoefficient;

		this->normalVector = normalVector;
	}
	MultiPathNodeInfoDiffuseScattering::MultiPathNodeInfoDiffuseScattering()
	{
		this->type = PropagationTypeStd::PropagationType::DiffuseScattering;

		this->upObjectType = -999;
		this->downObjectType = -999;

		this->widthCoefficientOfDiffuseLobe = 2;
		this->roughness = 0.0;
		this->thetai = 0.0;
		this->beta = 0.0;
		this->area = 0.0;
		this->scatteringCoefficient = 8.0;

	}
	MultiPathNodeInfoDiffuseScattering::~MultiPathNodeInfoDiffuseScattering()
	{
	}
}