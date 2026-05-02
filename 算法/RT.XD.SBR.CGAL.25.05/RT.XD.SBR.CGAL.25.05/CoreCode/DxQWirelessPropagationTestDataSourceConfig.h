#pragma once

#include<string>


namespace WirelessPropagationTestDataSourceConfigStd {

	class WirelessPropagationTestDataSourceConfig
	{
	public:

		/// <summary>
		/// 基站信息
		/// </summary>
		std::string inputTransmitterAntennaCsvFileName;

		/// <summary>
		/// 实例测量数据txt文件名，包括4列数据
		/// </summary>
		std::string actualMeasurementDataTxtFileName;

		/// <summary>
		/// 接收数据的抽样间隔，1表示使用全部数据，这个值不能小于1
		/// </summary>
		int samplingIntervalForReceivingData; 

		WirelessPropagationTestDataSourceConfig();
		~WirelessPropagationTestDataSourceConfig();

	private:

	};

}

