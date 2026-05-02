
#include "CalOfElectricFieldLossUnderCircularPolarization3D.h"


#include "CalculateDiffractionWaveLoss.h"
#include "CalculateDiffuseScatteringWaveLoss.h"
#include "CalculateReflectionWaveLoss.h"
#include "CalculateTransmissionWaveLoss.h"
#include "CalculateWaveLossCoefficientBase.h"
#include "CalculateWaveImpactResponseDBmComplex.h"

/// <summary>
/// 圆极化下的电场损耗计算
/// </summary>
namespace CalOfElectricFieldLossUnderCircularPolarization3DStd {


	/// <summary>
	/// 计算路径损耗，这里只能计算反射、绕射、漫散射损耗
	/// </summary>
	/// <param name="te"></param>
	/// <param name="tm"></param>
	/// <returns></returns>
	double CalculatePLbaseOnTeTm(
		double te_real, double te_imag,
		double tm_real, double tm_imag) {
		CalculateWaveLossCoefficientStd::Complex te(te_real, te_imag);
		CalculateWaveLossCoefficientStd::Complex tm(tm_real, tm_imag);
		return CalculateWaveLossCoefficientStd::CalculatePLbaseOnTeTm(te, tm);

	}

	/// <summary>
	/// 计算路径损耗，这里只能计算折射
	/// </summary>
	/// <param name="te"></param>
	/// <param name="tm"></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	/// <returns></returns>
	double CalculatePLbaseOnTeTmPlus(
		double te_real, double te_imag,
		double tm_real, double tm_imag,
		long long frequency,
		double thetai,
		double thetat,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2) {

		CalculateWaveLossCoefficientStd::Complex complexPermittivity1 = CalculateWaveLossCoefficientStd::CalculateComplexPermittivity(frequency, relativePermittivity1, conductivity1);
		CalculateWaveLossCoefficientStd::Complex complexPermittivity2 = CalculateWaveLossCoefficientStd::CalculateComplexPermittivity(frequency, relativePermittivity2, conductivity2);

		CalculateWaveLossCoefficientStd::Complex complexRefractiveIndexOfMedium1 = CalculateWaveLossCoefficientStd::CalculateComplexRefractiveIndexOfMedium(complexPermittivity1);
		CalculateWaveLossCoefficientStd::Complex complexRefractiveIndexOfMedium2 = CalculateWaveLossCoefficientStd::CalculateComplexRefractiveIndexOfMedium(complexPermittivity2);

		CalculateWaveLossCoefficientStd::Complex te(te_real, te_imag);
		CalculateWaveLossCoefficientStd::Complex tm(tm_real, tm_imag);
		return CalculateWaveLossCoefficientStd::CalculatePLbaseOnTeTmPlus(
			te, tm, thetai, thetat, complexRefractiveIndexOfMedium1, complexRefractiveIndexOfMedium2);

	}

	/// <summary>
	/// 计算天线辐射场的损耗
	/// </summary>
	double CalLossByAntennaRadiation(
		long long frequency,
		double distance,
		double antennaRadiationCoefficient,
		double relativePermittivity, double conductivity) {

		double loss1 = 32.447783221579070 + 20 * log10(frequency / 1000000) + 20 * log10(distance / 1000);

		if (distance<0.001) {
			loss1 = 0;
		}

		if (loss1 < 0) {
			loss1 = 0;
		}

		double u0_e0 = CalculateWaveLossCoefficientStd::MU_0 / CalculateWaveLossCoefficientStd::EPSILON_0;

		double lossANp = conductivity / 2 * sqrt(u0_e0 / relativePermittivity);

		double a0 = 20 * log10(exp(1));
		double loss2 = a0 * lossANp * distance;
		if (loss2 < 0) {
			loss2 = 0;
		}

		double loss3 = 10 * log10(relativePermittivity);
		if (loss3 < 0) {
			loss3 = 0;
		}

		double loss4 = -20 * log10(antennaRadiationCoefficient);

		double antennaRadiationLoss = loss1 + loss2 + loss3 + loss4;

		return antennaRadiationLoss;
	}

	//计算直射电场损耗
	double CalLossByDirectField(
		long long frequency,
		double pre_distance, double distance,
		double relativePermittivity, double conductivity) {

		if (pre_distance < CalculateWaveLossCoefficientStd::EPSILON) {
			return 0.0;
		}

		double w = 2 * CalculateWaveLossCoefficientStd::PI * frequency;

		double c1 = CalculateWaveLossCoefficientStd::MU_0 * w;
		double c2 = conductivity;
		double t3 = sqrt(c1 * c2);

		double coefficient = pre_distance *exp(-distance * t3) / (pre_distance + distance);

		double directionWaveLoss = -20. * log10(coefficient);

		if (directionWaveLoss > CalculateWaveLossCoefficientStd::MAX_PATH_LOSS) {
			directionWaveLoss = CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		}

		return directionWaveLoss;

	}

	//计算反射电场损耗
	double CalLossByReflectionField(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2) {

		double te_real, te_imag;
		double tm_real, tm_imag;

		CalculateWaveLossCoefficientStd::CalculateReflectionWaveCoefficient(frequency, thetai,
			relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			te_real, te_imag, tm_real, tm_imag);

		double pl = CalculatePLbaseOnTeTm(te_real, te_imag, tm_real, tm_imag);

		return pl;

	}

	//计算折射电场损耗
	double CalLossByTransmissionField(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2) {

		double te_real, te_imag;
		double tm_real, tm_imag;

		double thetat;
		if (!CalculateWaveLossCoefficientStd::CalculateTransmissionWaveCoefficient(frequency, thetai,
			relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			te_real, te_imag, tm_real, tm_imag, thetat)) {

			return CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		}

		double pl = CalculatePLbaseOnTeTmPlus(te_real, te_imag, tm_real, tm_imag,
			frequency, thetai, thetat,
			relativePermittivity1, conductivity1,
			relativePermittivity2, conductivity2);	

		return pl;
	}

	//计算绕射电场损耗
	double CalLossByDiffractionField(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity) {

		double te_real, te_imag;
		double tm_real, tm_imag;

		if (!CalculateWaveLossCoefficientStd::CalculateDiffractionWaveCoefficient(frequency,
			beta0, phi1, phi2, phiE,
			relativePermittivity, conductivity,
			te_real, te_imag, tm_real, tm_imag)) {
			return CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		}

		double pl = CalculatePLbaseOnTeTm(te_real, te_imag, tm_real, tm_imag);

		return pl;

	}


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
		double conductivity) {

		double te_real, te_imag;
		double tm_real, tm_imag;

		if (!CalculateWaveLossCoefficientStd::CalculateDiffuseScatteringCoefficient(roughness, frequency,
			thetai, thetar, ar, s, coefficient,
			relativePermittivity, conductivity,
			te_real, te_imag, tm_real, tm_imag)) {
			return CalculateWaveLossCoefficientStd::MAX_PATH_LOSS;
		}

		double pl = CalculatePLbaseOnTeTm(te_real, te_imag, tm_real, tm_imag);

		return pl;

	}

}