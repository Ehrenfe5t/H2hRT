#pragma once

#include<string>

namespace DataInputCsvFileParameterConfigStd {

	class DataInputCsvFileParameterConfig
	{
	public:
		std::string inputMaterialTableCsvFileName;
		std::string inputScenarioCorner3DCsvFileName;
		std::string inputScenarioPoint3DCsvFileName;
		std::string inputScenarioTriangle3DCsvFileName;
		std::string inputTransmittingAntennaDatabaseJsonFileName;

		DataInputCsvFileParameterConfig();
		~DataInputCsvFileParameterConfig();

	private:

	};

}

