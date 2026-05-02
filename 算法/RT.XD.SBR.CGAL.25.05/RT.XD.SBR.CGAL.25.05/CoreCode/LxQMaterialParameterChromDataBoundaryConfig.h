#pragma once

#include"LxQMaterialParameterChromDataBoundary.h"

#include<string>

namespace MaterialParameterChromDataBoundaryConfigStd {

	class MaterialParameterChromDataBoundaryConfig
	{
	public:

		/// <summary>
		/// 材质类型
		/// </summary>
		int typeNumber;
		/// <summary>
		/// 频率
		/// </summary>
		double frequency;

		MaterialParameterChromDataBoundaryStd::MaterialParameterChromDataBoundary materialParameterChromDataBoundary;

		MaterialParameterChromDataBoundaryConfig();
		~MaterialParameterChromDataBoundaryConfig();

	private:

	};


}