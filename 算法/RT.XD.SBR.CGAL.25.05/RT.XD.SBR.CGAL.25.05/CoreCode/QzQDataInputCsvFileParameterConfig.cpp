#include "QzQDataInputCsvFileParameterConfig.h"


namespace DataInputCsvFileParameterConfigStd {

    DataInputCsvFileParameterConfig::DataInputCsvFileParameterConfig()
    {
		this->inputMaterialTableCsvFileName = "ScenarioMaterial.csv";
		this->inputTransmittingAntennaDatabaseJsonFileName = "TransmittingAntennaDatabase.json";
		this->inputScenarioCorner3DCsvFileName = "ScenarioAccelerateCorner3D.csv";
		this->inputScenarioPoint3DCsvFileName = "ScenarioAcceleratePoint3D.csv";
		this->inputScenarioTriangle3DCsvFileName = "ScenarioAccelerateTriangle3D.csv";

    }

	DataInputCsvFileParameterConfig::~DataInputCsvFileParameterConfig()
	{
	}

}