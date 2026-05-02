#pragma once




namespace MaterialParameterChromDataBoundaryStd {

	class MaterialParameterChromDataBoundary
	{
	public:

		/// <summary>
		/// 相对介电常数
		/// </summary>
		double relativePermittivityMin;

		/// <summary>
		/// 相对介电常数
		/// </summary>
		double relativePermittivityMax;
		/// <summary>
		/// 电导率
		/// </summary>
		double conductivityMin;

		/// <summary>
		/// 电导率
		/// </summary>
		double conductivityMax;
		MaterialParameterChromDataBoundary();
		~MaterialParameterChromDataBoundary();

	private:

	};

}

