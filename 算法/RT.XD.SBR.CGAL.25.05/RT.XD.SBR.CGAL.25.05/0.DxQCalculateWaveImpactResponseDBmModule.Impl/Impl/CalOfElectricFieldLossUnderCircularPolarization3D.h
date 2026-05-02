#pragma once



namespace CalOfElectricFieldLossUnderCircularPolarization3DStd {

	/// <summary>
	/// 计算天线辐射场的损耗
	/// </summary>
	double CalLossByAntennaRadiation(
		long long frequency,
		double distance,
		double antennaRadiationCoefficient,
		double relativePermittivity, double conductivity);

	//计算直射电场损耗
	double CalLossByDirectField(
		long long frequency,
		double pre_distance, double distance,
		double relativePermittivity, double conductivity);

	//计算反射电场损耗
	double CalLossByReflectionField(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2);

	//计算折射电场损耗
	double CalLossByTransmissionField(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2);

	//计算绕射电场损耗
	double CalLossByDiffractionField(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity);


	//计算漫散射射电场损耗
	double CalLossByDiffuseScatteringField(
		double roughness,
		long long frequency,
		double thetai,
		double thetar,
		double ar,
		double s,
		double coefficient,
		double relativePermittivity,
		double conductivity);

}