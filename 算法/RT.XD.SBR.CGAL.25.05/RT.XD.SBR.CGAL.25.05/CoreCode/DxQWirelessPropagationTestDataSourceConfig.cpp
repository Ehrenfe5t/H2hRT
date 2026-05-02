
#include"DxQWirelessPropagationTestDataSourceConfig.h"

namespace WirelessPropagationTestDataSourceConfigStd {

	WirelessPropagationTestDataSourceConfig::WirelessPropagationTestDataSourceConfig()
	{
		this->samplingIntervalForReceivingData = 1;
		this->inputTransmitterAntennaCsvFileName = "TransmitterAntenna.csv";
		this->actualMeasurementDataTxtFileName = "ActualMeasurementData.txt";
	}

	WirelessPropagationTestDataSourceConfig::~WirelessPropagationTestDataSourceConfig()
	{
	}

}