#pragma once

#include <cmath>
#include "CalculateWaveLossCoefficientConstant.h"

namespace CalculateWaveLossCoefficientStd {
	struct Complex {
		double real;
		double imag;

		Complex(double r = 0, double i = 0) : real(r), imag(i) {}

		Complex Ni() const {
			return Complex(-real, -imag);
		}

		/// <summary>
		/// 计算幅度
		/// </summary>
		/// <returns></returns>
		double Abs() const {
			return sqrt(real * real + imag * imag);
		}

		/// <summary>
		/// 计算相位
		/// </summary>
		/// <returns></returns>
		double Arg() const {
			double angle = std::atan2(imag, real);
			if (angle < 0) angle += TWO_PI;
			return angle;
		}

		Complex Sqrt() const {
			double r = Abs();
			double theta = Arg();
			double real = std::sqrt(r) * std::cos(theta / 2.0);
			double imag = std::sqrt(r) * std::sin(theta / 2.0);
			return Complex(real, imag);
		}



	};

	//Complex operator+ (double scalar, const Complex& c);
	//Complex operator- (double scalar, const Complex& c);
	//Complex operator/ (double scalar, const Complex& c);
	//Complex operator+ (const Complex& c, double scalar);
	//Complex operator- (const Complex& c, double scalar);
	//Complex operator/ (const Complex& c, double scalar);

	Complex operator* (const Complex& c, double scalar);
	Complex operator* (double scalar, const Complex& c);


	Complex operator+ (const Complex& c1, const Complex& c2);
	Complex operator- (const Complex& c1, const Complex& c2);
	Complex operator* (const Complex& c1, const Complex& c2);
	Complex operator/ (const Complex& c1, const Complex& c2);

	Complex Exp(double d);
	Complex ExpComplex(const Complex& d);


}