#pragma once


namespace NumericalMethodParameterConfigStd {

	class NumericalMethodParameterConfig
	{
	public:

		/// <summary>
		/// 数值法计算时，最大的迭代次数
		/// </summary>
		int numericalMethodMaxLevelOfGuess;

		/// <summary>
		/// 数值法计算时，单次猜测的值的数量
		/// </summary>
		int numericalMethodNumbersOfGuess;

		NumericalMethodParameterConfig();
		~NumericalMethodParameterConfig();

	private:

	};


}