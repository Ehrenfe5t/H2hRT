
#include "CalculateDiffuseScatteringWaveLoss.h"
#include "CalculateWaveLossCoefficientBase.h"
#include "CalculateReflectionWaveLoss.h"

namespace CalculateWaveLossCoefficientStd {

	/// <summary>
	/// 基于Rayleigh criterion model判断是否是粗糙的
	/// </summary>
	/// <returns></returns>
	bool RayleighCriterionModel(
		double roughness,
		long long frequency,
		double thetai,
		double coefficient) {

		//波长
		double wavelength = C / frequency;

		double roughnessC = wavelength / (coefficient * cos(thetai));
		if (roughness > roughnessC) {
			return true;
		}
		else {
			return false;
		}

	}

	bool CalculateDiffuseScatteringWaveLoss(
		double roughness,
		long long frequency,
		double thetai,
		double thetar,
		double ar,
		double s,
		double coefficient,
		double relativePermittivity,
		double conductivity,
		Complex& te,
		Complex& tm) {

		//if (!RayleighCriterionModel(roughness, frequency, thetai, coefficient)) {
		//	return false;
		//}

		double relativePermittivity1 = 1.0;
		double conductivity1 = 0.0;
		double relativePermittivity2 = relativePermittivity;
		double conductivity2 = conductivity;

		Complex r_te, r_tm;
		CalculateReflectionWaveCoefficient(frequency,
			thetai, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_te.real, r_te.imag, r_tm.real, r_tm.imag);

		double dsd = s * pow(0.5*(1+cos(thetar)), ar);
		te = r_te * dsd;
		tm = r_tm * dsd;
		return true;

	}


	bool CalculateDiffuseScatteringCoefficient(
		double roughness,
		long long frequency,
		double thetai,
		double thetar,
		double ar,
		double s,
		double coefficient,
		double relativePermittivity,
		double conductivity,
		double& te_real, double& te_imag,
		double& tm_real, double& tm_imag) {

		Complex te, tm;
		if (!CalculateDiffuseScatteringWaveLoss(roughness, frequency, thetai, thetar, ar, s, coefficient, relativePermittivity, conductivity, te, tm)) {
			return false;
		}

		te_real = te.real;
		te_imag = te.imag;
		tm_real = tm.real;
		tm_imag = tm.imag;
		return true;

	}

}