#pragma once

#include"DxQWirelessPropagationTestDataSourceConfig.h"
#include<string>
#include<vector>

namespace OptimizationMaterialParameterConfigStd {
	

	class OptimizationMaterialParameterConfig
	{
	public:

		/// <summary>
		/// 材质输入
		/// </summary>
		std::string inputMaterialTableCsvFileName;

		/// <summary>
		/// 数据源
		/// </summary>
		std::vector<WirelessPropagationTestDataSourceConfigStd::WirelessPropagationTestDataSourceConfig> dataSource;

		/// <summary>
		/// 输出文件目录
		/// </summary>
		std::string outputDirectoryFileName;


		OptimizationMaterialParameterConfig();
		~OptimizationMaterialParameterConfig();

	private:

	};



}