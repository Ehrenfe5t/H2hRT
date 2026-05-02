
#include"QzQDiffuseScatteringParameter.h"

namespace DiffuseScatteringParameterStd {


	DiffuseScatteringParameter::DiffuseScatteringParameter()
	{
		/// <summary>
		/// 定向散射波瓣系数
		/// </summary>
		this->diffuseScatteringAr = 2;

		/// <summary>
		/// 散射系数
		/// </summary>
		this->diffuseScatteringCoefficient = 0.5;

		/// <summary>
		/// 毫米波瑞利判据范围8~32
		/// </summary>
		this->diffuseScatteringRayleighRange = 8;
	}

	DiffuseScatteringParameter::~DiffuseScatteringParameter()
	{
	}

}