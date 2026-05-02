#pragma once


#include"DxQTransmittingAntennaJsonFile.h"

namespace TransmittingAntennaDatabaseJsonFileStd {

	class TransmittingAntennaDatabaseJsonFile
	{
	public:

		std::string inputAntennaPatternDatabaseJsonFileName;
		std::string inputPolarization3DDatabaseJsonFileName;
		std::vector<TransmittingAntennaJsonFileStd::TransmittingAntennaJsonFile> transmittingAntennaJsonFiles;

		TransmittingAntennaDatabaseJsonFile();
		~TransmittingAntennaDatabaseJsonFile();

	private:

	};


}