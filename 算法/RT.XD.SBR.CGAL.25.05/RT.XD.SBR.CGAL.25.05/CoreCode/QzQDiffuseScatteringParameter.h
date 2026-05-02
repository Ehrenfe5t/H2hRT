#pragma once



namespace DiffuseScatteringParameterStd {

	class DiffuseScatteringParameter
	{
	public:
		/// <summary>
		/// 定向散射波瓣系数
		/// </summary>
		int diffuseScatteringAr;

		/// <summary>
		/// 散射系数
		/// </summary>
		double diffuseScatteringCoefficient;

		/// <summary>
		/// 毫米波瑞利判据范围8~32
		/// </summary>
		double diffuseScatteringRayleighRange;
		DiffuseScatteringParameter();
		~DiffuseScatteringParameter();

	private:

	};


}