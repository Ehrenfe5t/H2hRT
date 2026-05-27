// 文件目标：
// - 定义复数和常用复数运算。
//
// 主要功能：
// - 提供 Complex 基础类型及四则运算、共轭、模、辐角等操作；
// - 实现复指数函数（用于电磁波相位旋转）；
// - 提供与 std::complex 的双向转换以复用标准库功能。

#pragma once

#include <cmath>
#include <complex>

namespace rt {

/// <summary>
/// 复数结构，用于电磁场相位计算。
/// </summary>
struct Complex {
    double re = 0.0;
    double im = 0.0;

    Complex() = default;
    Complex(double r, double i) : re(r), im(i) {}

    /// <summary>获取实部。</summary>
    double Real() const { return re; }
    /// <summary>获取虚部。</summary>
    double Imag() const { return im; }

    /// <summary>模的平方，避免开方。</summary>
    double NormSq() const { return re * re + im * im; }
    /// <summary>模（幅度）。</summary>
    double Norm() const { return std::sqrt(NormSq()); }
    /// <summary>辐角（弧度）。</summary>
    double Arg() const { return std::atan2(im, re); }

    /// <summary>共轭复数。</summary>
    Complex Conj() const { return Complex(re, -im); }

    Complex operator+(const Complex& o) const { return Complex(re + o.re, im + o.im); }
    Complex operator-(const Complex& o) const { return Complex(re - o.re, im - o.im); }
    Complex operator*(const Complex& o) const {
        return Complex(re * o.re - im * o.im, re * o.im + im * o.re);
    }
    Complex operator/(const Complex& o) const {
        double denom = o.re * o.re + o.im * o.im;
        return Complex((re * o.re + im * o.im) / denom, (im * o.re - re * o.im) / denom);
    }
    Complex operator*(double s) const { return Complex(re * s, im * s); }
    Complex operator/(double s) const { return Complex(re / s, im / s); }
    Complex& operator+=(const Complex& o) { re += o.re; im += o.im; return *this; }
    Complex& operator-=(const Complex& o) { re -= o.re; im -= o.im; return *this; }
    Complex& operator*=(const Complex& o) { *this = *this * o; return *this; }
    Complex& operator*=(double s) { re *= s; im *= s; return *this; }
};

/// <summary>标量乘以复数的交换律（左乘）。</summary>
inline Complex operator*(double s, const Complex& c) { return c * s; }

/// <summary>
/// 复数平方根，返回主值。
/// </summary>
/// <param name="c">输入复数。</param>
/// <returns>平方根的主值。</returns>
inline Complex Sqrt(const Complex& c) {
    double mag = c.Norm();
    double r = std::sqrt(0.5 * (mag + c.re));
    double i = std::copysign(std::sqrt(0.5 * (mag - c.re)), c.im);
    return Complex(r, i);
}

/// <summary>
/// 复指数函数 e^c。
/// </summary>
/// <param name="c">指数参数。</param>
/// <returns>e^(re + i*im) = e^re * (cos(im) + i*sin(im))。</returns>
inline Complex Exp(const Complex& c) {
    double scalar = std::exp(c.re);
    return Complex(scalar * std::cos(c.im), scalar * std::sin(c.im));
}

/// <summary>
/// 纯旋转复指数 e^(i*phaseRad)，用于相位旋转。
/// </summary>
/// <param name="phaseRad">相位角（弧度）。</param>
/// <returns>单位幅度的旋转复数。</returns>
inline Complex CExp(double phaseRad) {
    return Complex(std::cos(phaseRad), std::sin(phaseRad));
}

/// <summary>从 std::complex 转换为本系统的 Complex 类型。</summary>
inline Complex FromStdComplex(const std::complex<double>& sc) {
    return Complex(sc.real(), sc.imag());
}

/// <summary>从本系统的 Complex 转换为 std::complex 类型。</summary>
inline std::complex<double> ToStdComplex(const Complex& c) {
    return std::complex<double>(c.re, c.im);
}

} // namespace rt
