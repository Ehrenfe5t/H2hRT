#pragma once

#include"HdQConfig.h"
#include"DxQTransmittingAntenna.h"

#include<vector>

namespace TransmittingAntennaDatabaseStd {


	INTERFACE_API void Add(const TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna);

	INTERFACE_API void AddRange(const std::vector<TransmittingAntennaStd::TransmittingAntenna>& transmittingAntennas);

	INTERFACE_API bool FindTransmittingAntenna(int transmittingAntennaId, TransmittingAntennaStd::TransmittingAntenna& transmittingAntenna);
}

