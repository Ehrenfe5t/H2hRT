


#include<math.h>
#include"HdQCalDBm.h"

#include"QzQGlobalConstant.h"

namespace CalDBmStd {

	double GetLossDbAttenuationOfElectromagneticWavesInAir(double f, double d) {
		double res = 20 * log10(f / 1e6) + 20 * log10(d / 1000) + 32.4;
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
		if (mw < 1e-34) {
			return powerThreshold;
		}
		return 10.0 * log10(mw);
	}


	double GetDBmByW(double w, double powerThreshold) {
		return GetDBmByMw(1000.0*w, powerThreshold);
	}

	/// <summary>
	/// 计算电场的幅值，这里的power是W,
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	double GetFieldAmplitudeByPower(double power) {
		return sqrt(power * GlobalConstantStd::FreeSpaceCharacteristicImpedance);
	}

}