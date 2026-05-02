
#include"LxQMultiPathNodeInfoDiffraction.h"

namespace MultiPathNodeInfoStd {
	MultiPathNodeInfoDiffraction::MultiPathNodeInfoDiffraction(
		int upObjectType, int downObjectType,
		double beta, double phiE, double phi1, double phi2,
		const Point3DStd::Point3D& location,
		const Point3DStd::Point3D& normalVector)
	{
		this->type = PropagationTypeStd::PropagationType::Diffraction;
		this->location = location;

		this->upObjectType = upObjectType;
		this->downObjectType = downObjectType;

		this->beta = beta;
		this->phiE = phiE;
		this->phi1 = phi1;
		this->phi2 = phi2;

		this->normalVector = normalVector;
	}
	MultiPathNodeInfoDiffraction::MultiPathNodeInfoDiffraction()
	{
		this->type = PropagationTypeStd::PropagationType::DiffuseScattering;

		this->upObjectType = -999;
		this->downObjectType = -999;

		this->beta = 0.0;
		this->phiE = 0.0;
		this->phi1 = 0.0;
		this->phi2 = 0.0;

	}
	MultiPathNodeInfoDiffraction::~MultiPathNodeInfoDiffraction()
	{
	}
}