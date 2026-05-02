#pragma once

#include"HdQAntennaSIMO.h"

namespace AntennaMIMOStd {

	class AntennaMIMO
	{
	public:

		std::vector<AntennaSIMOStd::AntennaSIMO> antennaSIMOs;

		AntennaMIMO();
		~AntennaMIMO();

	private:

	};

	void UpdataTransmittingAntennaDatabase(const AntennaMIMOStd::AntennaMIMO& antennaMIMO);

}