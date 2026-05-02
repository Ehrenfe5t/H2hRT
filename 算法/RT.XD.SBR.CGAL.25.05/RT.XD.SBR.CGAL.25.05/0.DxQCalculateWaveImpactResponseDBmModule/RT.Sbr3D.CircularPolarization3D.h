#pragma once

#include "Antenna.h"
#include "Sbr3DFindPathConfig.h"
#include "Material.h"
#include "Scenario3D.h"
#include "ElectricFieldPath.h"


extern "C" _declspec(dllexport) RtoiOutputInformation * RTSbr3DCircularPolarization3DPtr(
	const Sbr3DFindPathConfig & config,
	const MaterialSet & materialSet,
	const Scenario3D & scenario,
	const AntennaPolarization3DModel & antennaPolarization3DModel,
	const AntennaRadiationPattern3DModel & antennaRadiationPattern3DModel,
	const TransmitterAntenna & transmitterAntenna,
	bool& success);

extern "C" _declspec(dllexport) void FreeMemoryRTSbr3DCircularPolarization3DPtr();
