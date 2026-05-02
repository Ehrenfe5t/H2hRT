#pragma once

#include"CalculateWaveImpactResponseDBmComplex.h"

namespace CalculateWaveLossCoefficientStd {

	double GetLossDbAttenuationOfElectromagneticWavesInAir(long long frequency, double distance);

	double GetMwByDBm(double dBm);

	double GetWByDBm(double dBm);

	double GetDBmByMw(double mw, double powerThreshold);

	double GetDBmByW(double w, double powerThreshold);

	double AddDBm(double dBm1, double dBm2);

	Complex CalculateComplexRefractiveIndexOfMedium(const Complex& complexPermittivity);

	Complex CalculateComplexPermittivity(
		long long frequency,
		double relativePermittivity,
		double conductivity);

	bool CalculateThetat(double thetai,
		const Complex& complexRefractiveIndexOfMedium1,
		const Complex& complexRefractiveIndexOfMedium2,
		double& thetat);


	double CalculatePLbaseOnTeTm(
		const Complex& te,
		const Complex& tm);


	double CalculatePLbaseOnTeTmPlus(
		const Complex& te,
		const Complex& tm,
		double thetai,
		double thetat,
		const Complex& complexRefractiveIndexOfMedium1,
		const Complex& complexRefractiveIndexOfMedium2);

}