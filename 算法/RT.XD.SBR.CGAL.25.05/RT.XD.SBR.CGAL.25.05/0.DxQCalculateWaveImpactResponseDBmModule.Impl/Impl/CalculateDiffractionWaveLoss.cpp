
#include "CalculateDiffractionWaveLoss.h"

#include<vector>

#include"CalculateWaveLossCoefficientBase.h"
#include"CalculateReflectionWaveLoss.h"

//重构菲涅尔积分

namespace CalculateWaveLossCoefficientStd {

	const double eps = 1e-6;
	const double max_x_value = 100;

}


namespace CalculateWaveLossCoefficientStd {


	/// <summary>
	/// 积分的结果，计算速度太慢不适合大规模计算
	/// </summary>
	/// <param name="x"></param>
	/// <returns></returns>
	Complex FresnelFunction(double x) {
		if (x < 0.0001) {
			Complex temp1 = Exp(PI / 4.0);
			Complex temp2 = Exp(-PI / 4.0);
			Complex temp3 = Exp(x + PI / 4.0);

			Complex Fx = (sqrt(PI * x) - temp1 * 2 * x - 2 / 3 * x * x * temp2) * temp3;
			return Fx;
		}
		else if (x >= 0.0001 && x <= 10) {
			//原方法，使用循环进行积分，效率较低
			double Fx_re = sqrt(PI * x) * cos(x + PI / 4);
			double Fx_im = sqrt(PI * x) * sin(x + PI / 4);
			double max_x_value = sqrt(x);
			double step = 1e-4;
			
			for (double dx = 0; dx <= max_x_value; dx += step) {
				Fx_re = Fx_re - 2 * max_x_value * cos(x + PI / 2 - dx * dx) * step;
				Fx_im = Fx_im - 2 * max_x_value * sin(x + PI / 2 - dx * dx) * step;
			}
			
			Complex Fx(Fx_re, Fx_im);
			return Fx;


		}
		else {
			double Fx_re = 1 - 3 / (4 * x * x) + 75 / (16 * x * x * x * x);
			double Fx_im = 1 / (2 * x) - 1 * 15 / (8 * x * x * x);
			Complex Fx(Fx_re, Fx_im);
			return Fx;
		}

	}



	void Cala1234(double p_2_n, double phi1_add_phi2, double phi2_sub_phi1,
		double& a1, double& a2, double& a3, double& a4) {

		a1 = (PI - phi2_sub_phi1) / p_2_n;
		a2 = (PI + phi2_sub_phi1) / p_2_n;
		a3 = (PI - phi1_add_phi2) / p_2_n;
		a4 = (PI + phi1_add_phi2) / p_2_n;
	}

	double Cot(double x) {

		return 1.0 / tan(x);

		//if (x > 2 * PI) {
		//	return Cot(x - 2 * PI);
		//}
		//
		//if (x < -2 * PI) {
		//	return Cot(x + 2 * PI);
		//}
		//
		//const double EPS_MIN_ARG = 0.0016*4;
		//
		//if (x<0 && x>-EPS_MIN_ARG) {
		//	x = -EPS_MIN_ARG;
		//}
		//else if (x > 0 && x < EPS_MIN_ARG) {
		//	x = EPS_MIN_ARG;
		//}
		//
		//if (abs(x - PI) < EPS_MIN_ARG) {
		//
		//	x = PI - EPS_MIN_ARG;
		//}
		//
		//
		//if (abs(x + PI) < EPS_MIN_ARG) {
		//
		//	x = - PI - EPS_MIN_ARG;
		//}
		//
		//
		//if (abs(x - 2*PI) < EPS_MIN_ARG) {
		//
		//	x = 2 * PI - EPS_MIN_ARG;
		//}
		//
		//
		//if (abs(x + 2 * PI) < EPS_MIN_ARG) {
		//
		//	x = -2 * PI - EPS_MIN_ARG;
		//}
		//
		//
		//return 1.0 / tan(x);
	}


	Complex CalD0(double p_2_n, double k, double sin_beta0) {
		auto temp1 = Exp(-PI / 4.0);
		auto temp2 = sqrt(2 * PI * k);
		Complex D0 = temp1 / (p_2_n * temp2 * sin_beta0);
		return D0;
	}

	//Complex CalD1234_(double a1,double L, double k, double p_2_n, double sin_beta0) {
	//
	//	double sin_a1 = sin(a1);
	//
	//	if (sin_a1 > EPSILON) {
	//		double x1 = sin_a1 * sin_a1 * L * k * p_2_n;
	//
	//		Complex Fx1 = FresnelFunction(x1);
	//
	//		Complex D0 = CalD0(p_2_n, k, sin_beta0);
	//
	//		double c1 = Cot(a1);
	//
	//		Complex D1 = D0 * c1 * Fx1;
	//
	//		return D1;
	//	}
	//	else
	//	{
	//		double x1 = L * k * p_2_n;
	//
	//		Complex Fx1 = FresnelFunction(x1);
	//
	//		Complex D0 = CalD0(p_2_n, k, sin_beta0);
	//
	//		Complex D1 = D0 * Fx1;
	//
	//		return D1;
	//	}
	//
	//	
	//}

	void CalD1234(double n, double k, double sin_beta0, double phi1, double phi2,
		Complex& D1, Complex& D2, Complex& D3, Complex& D4) {

		double phi1_add_phi2 = phi1 + phi2;
		double phi2_sub_phi1 = phi2 - phi1;
		double p_2_n = 2 * n;

		double a1, a2, a3, a4;
		Cala1234(p_2_n, phi1_add_phi2, phi2_sub_phi1, a1, a2, a3, a4);

		double L = sin_beta0 * sin_beta0;

		double sin_a1 = sin(a1);
		double sin_a2 = sin(a2);
		double sin_a3 = sin(a3);
		double sin_a4 = sin(a4);

		double x1 = sin_a1 * sin_a1 * L * k * p_2_n;
		double x2 = sin_a2 * sin_a2 * L * k * p_2_n;
		double x3 = sin_a3 * sin_a3 * L * k * p_2_n;
		double x4 = sin_a4 * sin_a4 * L * k * p_2_n;

		Complex Fx1 = FresnelFunction(x1);
		Complex Fx2 = FresnelFunction(x2);
		Complex Fx3 = FresnelFunction(x3);
		Complex Fx4 = FresnelFunction(x4);

		Complex D0 = CalD0(p_2_n, k, sin_beta0);

		double c1 = Cot(a1);
		double c2 = Cot(a2);
		double c3 = Cot(a3);
		double c4 = Cot(a4);

		D1 = D0 * c1 * Fx1;
		D2 = D0 * c2 * Fx2;
		D3 = D0 * c3 * Fx3;
		D4 = D0 * c4 * Fx4;

		//D1 = CalD1234_(a1, L, k, p_2_n, sin_beta0);
		//D2 = CalD1234_(a2, L, k, p_2_n, sin_beta0);
		//D3 = CalD1234_(a3, L, k, p_2_n, sin_beta0);	
		//D4 = CalD1234_(a4, L, k, p_2_n, sin_beta0);
	}


	double CalculateThetaIn(double n, double phi1, double phi2) {
		double thetain;
		if (n >= 1.5) {
			if (phi1 <= (n - 1) * PI) {
				if (phi2 <= (PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 >= PI) {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
				else {
					thetain = abs(phi2 + (n - 2.5) * PI);
				}
			}
			else if (phi1 > PI) {
				if (phi2 <= PI && phi2 <= abs((2 * n - 1) * PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 >= PI) {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
				else {
					thetain = abs(phi2 + (n - 2.5) * PI);
				}

			}
			else {

				if (phi2 < (PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 < abs((2 * n - 1) * PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 > PI) {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
				else {
					thetain = abs(phi2 + (n - 2.5) * PI);
				}
			}
		}
		else {

			if (phi1 <= (n - 1) * PI) {
				if (phi2 < (PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 > PI) {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
				else {
					thetain = abs(phi2 - (n - 0.5) * PI);
				}
			}
			else if (phi1 >= PI) {
				if (phi2 <= abs((2 * n - 1) * PI - phi1)) {
					thetain = abs(0.5 * PI - phi2);
				}
				else if (phi2 > PI) {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
				else {
					thetain = abs(phi2 - (n - 0.5) * PI);
				}
			}
			else {
				if (phi2 < PI - phi1) {
					thetain = abs(0.5 * PI - phi2);
				}
				else {
					thetain = abs(0.5 * PI - (n * PI - phi2));
				}
			}
		}
		return thetain;
	}

	bool CalculateDiffractionWaveLossCheck(
		double beta0, double phi1, double phi2, double phiE) {

		if (phiE < MIN_ARG) {
			return false;
		}
		if (phiE > 2.0 * PI - MIN_ARG) {
			return false;
		}

		if (phi1 > phiE) {
			return false;
		}
		if (phi2 > phiE) {
			return false;
		}

		if (phi1 < MIN_ARG) {
			return false;
		}
		if (phi2 < MIN_ARG) {
			return false;
		}
		if (beta0 < PI / 90.0) {
			return false;
		}
		if (beta0 > 0.5 * PI + MIN_ARG) {
			return false;
		}
		return true;
	}

	/// <summary>
	/// 根据Luebbers公式计算衍射波损耗系数
	/// </summary>
	/// <param name="frequency"></param>
	/// <param name="beta0"></param>
	/// <param name="phi1"></param>
	/// <param name="phi2"></param>
	/// <param name="phiE"></param>
	/// <param name="relativePermittivity"></param>
	/// <param name="conductivity"></param>
	/// <returns></returns>
	bool CalculateDiffractionWaveLossLuebbers(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm) {

		if (!CalculateDiffractionWaveLossCheck(beta0, phi1, phi2, phiE)) {
			return false;
		}

		if (beta0 > 0.5 * PI) {
			beta0 = 0.5 * PI;
		}

		//确定0面和n面
		if (phi1 > phiE - phi1) {
			phi1 = phiE - phi1;
			phi2 = phiE - phi2;
		}

		double k = 2 * PI * frequency / C;
		double sin_beta0 = sin(beta0);
		double n = phiE / PI;
		Complex D1, D2, D3, D4;
		CalD1234(n, k, sin_beta0, phi1, phi2, D1, D2, D3, D4);

		double thetai0 = 0.5 * PI - phi1;

		double thetain = CalculateThetaIn(n, phi1, phi2);

		double relativePermittivity1 = 1.0;
		double conductivity1 = 0.0;
		double relativePermittivity2 = relativePermittivity;
		double conductivity2 = conductivity;

		Complex r_0_te, r_0_tm, r_n_te, r_n_tm;
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_0_te.real, r_0_te.imag, r_0_tm.real, r_0_tm.imag);
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_n_te.real, r_n_te.imag, r_n_tm.real, r_n_tm.imag);


		te = D1 + D2 + r_0_te * D3 + r_n_te * D4;
		tm = D1 + D2 + r_0_tm * D3 + r_n_tm * D4;

		return true;
	}

	/// <summary>
	/// 根据Holm公式计算衍射波损耗系数
	/// </summary>
	/// <param name="frequency"></param>
	/// <param name="beta0"></param>
	/// <param name="phi1"></param>
	/// <param name="phi2"></param>
	/// <param name="phiE"></param>
	/// <param name="relativePermittivity"></param>
	/// <param name="conductivity"></param>
	/// <returns></returns>
	bool CalculateDiffractionWaveLossHolm(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm) {

		if (!CalculateDiffractionWaveLossCheck(beta0, phi1, phi2, phiE)) {
			return false;
		}

		if (beta0 > 0.5 * PI) {
			beta0 = 0.5 * PI;
		}

		//确定0面和n面
		if (phi1 > phiE - phi1) {
			phi1 = phiE - phi1;
			phi2 = phiE - phi2;
		}

		double k = 2 * PI * frequency / C;
		double sin_beta0 = sin(beta0);
		double n = phiE / PI;
		Complex D1, D2, D3, D4;
		CalD1234(n, k, sin_beta0, phi1, phi2, D1, D2, D3, D4);

		double thetai0 = 0.5 * PI - phi1;

		double thetain = CalculateThetaIn(n, phi1, phi2);

		double relativePermittivity1 = 1.0;
		double conductivity1 = 0.0;
		double relativePermittivity2 = relativePermittivity;
		double conductivity2 = conductivity;

		Complex r_0_te, r_0_tm, r_n_te, r_n_tm;
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_0_te.real, r_0_te.imag, r_0_tm.real, r_0_tm.imag);
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_n_te.real, r_n_te.imag, r_n_tm.real, r_n_tm.imag);

		te = D1 + r_0_te * r_n_te * D2 + r_0_te * D3 + r_n_te * D4;
		tm = D1 + r_0_tm * r_n_tm * D2 + r_0_tm * D3 + r_n_tm * D4;

		return true;
	}

	/// <summary>
	/// 根据Holm改进公式计算衍射波损耗系数
	/// [1]A New Heuristic UTD Diffraction Coefficient for Nonperfectly Conducting Wedges. Peter D. Holm
	/// [2]Improvements to Diffraction Coefficient for Non-Perfectly Conducting Wedges. Hassan M. El-Sallabi and Pertti Vainikainen
	/// [3]Heuristic UTD Diffraction Coefficient for Three-Dimensional Dielectric Wedges. Takahiro Hashimoto
	/// </summary>
	/// <param name="frequency"></param>
	/// <param name="beta0"></param>
	/// <param name="phi1"></param>
	/// <param name="phi2"></param>
	/// <param name="phiE"></param>
	/// <param name="relativePermittivity"></param>
	/// <param name="conductivity"></param>
	/// <returns></returns>
	bool CalculateDiffractionWaveLossHolmPlus(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		Complex& te, Complex& tm) {

		if (!CalculateDiffractionWaveLossCheck(beta0, phi1, phi2, phiE)) {
			return false;
		}

		if (beta0 > 0.5 * PI) {
			beta0 = 0.5 * PI;
		}

		//确定0面和n面
		if (phi1 > phiE - phi1) {
			phi1 = phiE - phi1;
			phi2 = phiE - phi2;
		}

		double k = 2 * PI * frequency / C;
		double sin_beta0 = sin(beta0);
		double n = phiE / PI;
		Complex D1, D2, D3, D4;
		CalD1234(n, k, sin_beta0, phi1, phi2, D1, D2, D3, D4);

		double thetai0 = 0.5 * PI - phi1;

		double thetain = CalculateThetaIn(n, phi1, phi2);

		double relativePermittivity1 = 1.0;
		double conductivity1 = 0.0;
		double relativePermittivity2 = relativePermittivity;
		double conductivity2 = conductivity;

		Complex r_0_te, r_0_tm, r_n_te, r_n_tm;
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_0_te.real, r_0_te.imag, r_0_tm.real, r_0_tm.imag);
		CalculateReflectionWaveCoefficient(frequency,
			thetai0, relativePermittivity1, conductivity1, relativePermittivity2, conductivity2,
			r_n_te.real, r_n_te.imag, r_n_tm.real, r_n_tm.imag);

		if (phi1 > (n - 1) * PI && phi1 < PI && phi2 < abs((2 * n - 1) * PI - phi1)) {
			double tao = 2.0 * sin(0.5 * phi1) * sin(0.5 * phi2);
			double tao_2 = tao * tao;
			double temp1 = sqrt(relativePermittivity - 1 + tao_2);
			double temp2 = relativePermittivity * tao;

			double epu_te = (temp2 - temp1) / (temp2 + temp1);
			double epu_tm = (tao - temp1) / (tao + temp1);

			te = D1 + D2 + epu_te * (D3 + D4);
			tm = D1 + D2 + epu_tm * (D3 + D4);

		}
		else if (phi1 > PI) {

			te = r_0_te * r_n_te * D1 + D2 + r_n_te * D3 + r_0_te * D4;
			tm = r_0_tm * r_n_tm * D1 + D2 + r_n_tm * D3 + r_0_tm * D4;
		}
		else {

			te = D1 + r_0_te * r_n_te * D2 + r_0_te * D3 + r_n_te * D4;
			tm = D1 + r_0_tm * r_n_tm * D2 + r_0_tm * D3 + r_n_tm * D4;
		}


		return true;
	}


	bool CalculateDiffractionWaveCoefficient(
		long long frequency,
		double beta0, double phi1, double phi2, double phiE,
		double relativePermittivity, double conductivity,
		double& te_real, double& te_imag,
		double& tm_real, double& tm_imag) {

		Complex te, tm;
		bool state = CalculateDiffractionWaveLossHolmPlus(
			frequency, beta0, phi1, phi2, phiE,
			relativePermittivity, conductivity,
			te, tm);
		te_real = te.real;
		te_imag = te.imag;
		tm_real = tm.real;
		tm_imag = tm.imag;
		if (!state) {
			te_real = 0.0;
			te_imag = 0.0;
			tm_real = 0.0;
			tm_imag = 0.0;
		}

		return state;
	}

}