#pragma once


namespace CalculateWaveLossCoefficientStd {



	void CalculateReflectionWaveCoefficient(
		long long frequency,
		double thetai,
		double relativePermittivity1, double conductivity1,
		double relativePermittivity2, double conductivity2,
		double& te_real, double& te_imag,
		double& tm_real, double& tm_imag);

}