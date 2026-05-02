
#include "CalculateWaveImpactResponseDBmComplex.h"


namespace CalculateWaveLossCoefficientStd {

	Complex operator* (double scalar, const Complex& c) {
		return Complex(c.real * scalar, c.imag * scalar);
	}

	Complex operator* (const Complex& c, double scalar) {
		return Complex(c.real * scalar, c.imag * scalar);
	}


	Complex operator+ (const Complex& c1, const Complex& c2) {
		return Complex(c1.real + c2.real, c1.imag + c2.imag);
	}
	Complex operator- (const Complex& c1, const Complex& c2) {
		return Complex(c1.real - c2.real, c1.imag - c2.imag);
	}
	Complex operator* (const Complex& c1, const Complex& c2) {
		return Complex(c1.real * c2.real - c1.imag * c2.imag, c1.real * c2.imag + c1.imag * c2.real);
	}
	Complex operator/ (const Complex& c1, const Complex& c2) {
		double denominator = c2.real * c2.real + c2.imag * c2.imag;
		return Complex((c1.real * c2.real + c1.imag * c2.imag) / denominator, (c1.imag * c2.real - c1.real * c2.imag) / denominator);
	}
	

	/// <summary>
	/// d代表角度，返回复数的e^(i*d)
	/// </summary>
	/// <param name="d"></param>
	/// <returns></returns>
	Complex Exp(double d) {

		return Complex(cos(d), sin(d));

	}


	Complex ExpComplex(const Complex& d) {
		double r = std::exp(d.real);
		double theta = d.Arg();
		double real = r * std::cos(theta);
		double imag = r * std::sin(theta);
		return Complex(real, imag);
	}
}