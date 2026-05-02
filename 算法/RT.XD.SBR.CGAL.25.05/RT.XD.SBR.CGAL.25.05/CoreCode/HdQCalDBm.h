#pragma once



namespace CalDBmStd {
	double GetLossDbAttenuationOfElectromagneticWavesInAir(double f, double d);
	/// <summary>
	/// 根据dBm计算mW
	/// </summary>
	/// <param name="dBm"></param>
	/// <returns></returns>
	double GetMwByDBm(double dBm);

	/// <summary>
	/// 根据dBm计算W
	/// </summary>
	/// <param name="dBm"></param>
	/// <returns></returns>
	double GetWByDBm(double dBm);


	double GetDBmByMw(double mw, double powerThreshold);


	double GetDBmByW(double w, double powerThreshold);

	double GetFieldAmplitudeByPower(double power);
}