

#include "CalculateTransmissionWaveLoss.h"
#include "CalculateWaveLossCoefficientBase.h"

namespace CalculateWaveLossCoefficientStd {


	void CalculateTransmissionWaveCoefficientBase(
		double thetai,
		double thetat,
		const Complex& complexRefractiveIndexOfMedium1,
		const Complex& complexRefractiveIndexOfMedium2,
		Complex& te,
		Complex& tm) {


		double cos_thetai = cos(thetai);
		double cos_thetat = cos(thetat);

		Complex temp1 = complexRefractiveIndexOfMedium1 * cos_thetai;
		Complex temp2 = complexRefractiveIndexOfMedium2 * cos_thetai;
		Complex temp3 = complexRefractiveIndexOfMedium1 * cos_thetat;
		Complex temp4 = complexRefractiveIndexOfMedium2 * cos_thetat;

		te = temp1 * 2.0 / (temp1 + temp4);
		tm = temp1 * 2.0 / (temp2 + temp3);

	}


	bool CalculateTransmissionWaveCoefficient(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2,
		double& te_real, double& te_imag,
		double& tm_real, double& tm_imag,
		double& thetat) {


		Complex complexPermittivity1 = CalculateComplexPermittivity(frequency, relativePermittivity1, conductivity1);
		Complex complexPermittivity2 = CalculateComplexPermittivity(frequency, relativePermittivity2, conductivity2);

		Complex complexRefractiveIndexOfMedium1 = CalculateComplexRefractiveIndexOfMedium(complexPermittivity1);
		Complex complexRefractiveIndexOfMedium2 = CalculateComplexRefractiveIndexOfMedium(complexPermittivity2);

		if (!CalculateThetat(thetai, complexRefractiveIndexOfMedium1, complexRefractiveIndexOfMedium2, thetat)) {

			te_real = 0.0;
			te_imag = 0.0;
			tm_real = 0.0;
			tm_imag = 0.0;

			return false;
		}

		Complex r_te, r_tm;
		CalculateTransmissionWaveCoefficientBase(thetai, thetat, complexRefractiveIndexOfMedium1, complexRefractiveIndexOfMedium2, r_te, r_tm);

		te_real = r_te.real;
		te_imag = r_te.imag;
		tm_real = r_tm.real;
		tm_imag = r_tm.imag;

		return true;
	}
}