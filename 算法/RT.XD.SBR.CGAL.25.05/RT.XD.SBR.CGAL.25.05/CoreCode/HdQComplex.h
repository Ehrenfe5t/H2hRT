#pragma once


#include<string>

namespace ComplexStd {
    /// <summary>
    /// 复数
    /// </summary>
    class Complex {
    public:
        /// <summary>
        /// 实部
        /// </summary>
        double real;
        /// <summary>
        /// 虚部
        /// </summary>
        double imag;

        Complex();

        Complex(const Complex& c);

        Complex(double r, double i);

        ~Complex();

        std::string ToString();
    };


}