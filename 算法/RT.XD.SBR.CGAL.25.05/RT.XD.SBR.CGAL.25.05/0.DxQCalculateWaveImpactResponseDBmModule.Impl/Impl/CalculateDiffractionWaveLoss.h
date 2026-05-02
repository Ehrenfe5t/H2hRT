#pragma once

#include"CalculateWaveImpactResponseDBmComplex.h"

namespace CalculateWaveLossCoefficientStd {


	Complex FresnelFunction(double x);


	void CalD1234(double n, double k, double sin_beta0, double phi1, double phi2,
		Complex& D1, Complex& D2, Complex& D3, Complex& D4);


	bool CalculateDiffractionWaveLossLuebbers(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm);


	bool CalculateDiffractionWaveLossHolm(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE, 
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm);


	bool CalculateDiffractionWaveLossHolmPlus(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm);

	bool CalculateDiffractionWaveCoefficient(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		double& te_real, double& te_imag,
		double& tm_real, double& tm_imag);

}