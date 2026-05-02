#pragma once



namespace CalculateWaveLossCoefficientStd {


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
		double& tm_real, double& tm_imag);

}