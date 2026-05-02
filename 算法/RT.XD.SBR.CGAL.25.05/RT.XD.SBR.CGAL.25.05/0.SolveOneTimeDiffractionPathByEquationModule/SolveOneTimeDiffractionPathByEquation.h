#pragma once

#include"0.SolveOneTimeDiffractionPathByEquationModule.Input.h"

#include<list>
//调用该模块默认已经初始化了场景的加速结构
namespace SolveOneTimeDiffractionPathByEquationStd {

	void SolveOneTimeDiffractionPathByEquation(
		double cornerRadius,
		const TransmitterAntenna& transmitterAntenna,
		const Scenario3D& scenario,
		const MaterialSet& materialSet,
		std::vector<std::list<std::vector<ElectricFieldNode>>>& result);

}