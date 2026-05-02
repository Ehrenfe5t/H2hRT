

#include "CalculateWaveLossCoefficientBase.h"


namespace CalculateWaveLossCoefficientStd {

	double GetLossDbAttenuationOfElectromagneticWavesInAir(long long frequency, double distance) {
		double res = 20 * log10(frequency / 1e6) + 20 * log10(distance / 1000) + 32.447783221579070;
		if (res <= 0) {
			return 0.0;
		}
		return res;
	}
	/// <summary>
	/// 根据dBm计算mW
	/// </summary>
	/// <param name="dBm"></param>
	/// <returns></returns>
	double GetMwByDBm(double dBm) {
		return pow(10.0, dBm / 10.0);
	}

	/// <summary>
	/// 根据dBm计算W
	/// </summary>
	/// <param name="dBm"></param>
	/// <returns></returns>
	double GetWByDBm(double dBm) {
		return GetMwByDBm(dBm) / 1000.0;
	}


	double GetDBmByMw(double mw, double powerThreshold) {
		if (mw < 1e-25) {
			return powerThreshold;
		}
		return 10.0 * log10(mw);
	}


	double GetDBmByW(double w, double powerThreshold) {
		return GetDBmByMw(1000.0 * w, powerThreshold);
	}



	double AddDBm(double dBm1, double dBm2) {
		double w1 = GetWByDBm(dBm1);
		double w2 = GetWByDBm(dBm2);
		double w = w1 + w2;
		return GetDBmByW(w, -MAX_PATH_LOSS);
	}


	/// <summary>
	/// 计算复数折射率
	/// </summary>
	/// <param name="complexPermittivity"></param>
	/// <returns></returns>
	Complex CalculateComplexRefractiveIndexOfMedium(const Complex& complexPermittivity) {
		return complexPermittivity.Sqrt();
	}

	/// <summary>
	/// 计算复数介电常数
	/// </summary>
	/// <returns></returns>
	Complex CalculateComplexPermittivity(
		long long frequency,
		double relativePermittivity,
		double conductivity) {

		double w = TWO_PI * frequency;
		double real = relativePermittivity * EPSILON_0;
		double imag = conductivity / w;
		Complex complexPermittivity(real, imag);
		return complexPermittivity;
	}

	/// <summary>
	/// 根据复数折射率计算折射角
	/// </summary>
	/// <param name="thetai"></param>
	/// <param name="complexRefractiveIndexOfMedium1"></param>
	/// <param name="complexRefractiveIndexOfMedium2"></param>
	/// <param name="thetat"></param>
	/// <returns></returns>
	bool CalculateThetat(double thetai,
		const Complex& complexRefractiveIndexOfMedium1,
		const Complex& complexRefractiveIndexOfMedium2,
		double& thetat) {


		// 计算折射率
		double n1 = complexRefractiveIndexOfMedium1.Abs();
		double n2 = complexRefractiveIndexOfMedium2.Abs();

		//计算折射角
		double sin_thetat = n1 * sin(thetai) / n2;
		if (sin_thetat > 1 || sin_thetat < -1) {
			return false;
		}
		thetat = asin(sin_thetat);
		return true;
	}


	/// <summary>
	/// 计算路径损耗，这里只能计算反射、绕射、漫散射损耗
	/// </summary>
	/// <param name="te"></param>
	/// <param name="tm"></param>
	/// <returns></returns>
	double CalculatePLbaseOnTeTm(
		const Complex& te,
		const Complex& tm) {

		double mod_te = te.Abs();
		double mod_tm = tm.Abs();
		double plx = 2.0 / (mod_te * mod_te + mod_tm * mod_tm);

		double PL = GetDBmByMw(plx, -MAX_PATH_LOSS);
		return PL;
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
		const Complex& te,
		const Complex& tm,
		double thetai,
		double thetat,
		const Complex& complexRefractiveIndexOfMedium1,
		const Complex& complexRefractiveIndexOfMedium2) {

		// 计算折射损耗时，由于分界面两侧的介质电参数不同所有 固有阻抗z不能消除
		Complex ccc = complexRefractiveIndexOfMedium2 * cos(thetat) / (complexRefractiveIndexOfMedium1 * cos(thetai));

		double cc = ccc.Abs();

		double mod_te = te.Abs();
		double mod_tm = tm.Abs();
		double plx = 2.0 / (mod_te * mod_te + mod_tm * mod_tm) / cc;
		double PL = GetDBmByMw(plx, -MAX_PATH_LOSS);

		return PL;

	}

}
