#pragma once


#include"0.Solve1SPathByIm3DModule.Input.h"

#include<list>

namespace Solve1SPathByIm3DStd {

	void Solve1SPathByIm3D(
		double diffuseScatteringMaxDiscreteSideLength,
		double diffuseScatteringAr,
		double diffuseScatteringCoefficient,
		double diffuseScatteringRayleighRange,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		std::vector<std::list<std::vector<ElectricFieldNode>>>& result);

}