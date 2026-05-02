
#include"HdQComplexOperate.h"
#include"LxQMathOperate.h"
#include"QzQGlobalConstant.h"

namespace ComplexStd {

    double Length(const Complex& c) {
        return sqrt(c.real * c.real + c.imag * c.imag);
    }

    /// <summary>
    /// 赋值操作 
    /// </summary>
    /// <param name="e1"></param>
    /// <param name="e2"></param>
    void AssignmentComplexComplex(ComplexStd::Complex& e1, const ComplexStd::Complex& e2) {
        e1.real = e2.real;
        e1.imag = e2.imag;
    }

    ComplexStd::Complex AddComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2) {

        ComplexStd::Complex res(c1.real + c2.real, c1.imag + c2.imag);
        return res;
    }
    ComplexStd::Complex AddComplexComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2, const ComplexStd::Complex& c3) {

        ComplexStd::Complex res(c1.real + c2.real + c3.real, c1.imag + c2.imag + c3.imag);
        return res;
    }

    ComplexStd::Complex SqrtComplex(const ComplexStd::Complex& c1) {
        double r, theta;
        ComplexStd::Complex res(0, 0);
        r = sqrt(c1.real * c1.real + c1.imag * c1.imag);
        if (1 == MathOperateStd::OneNumberIsZeroByEps(r)) {
            //std::cout << __FILE__ << "\tComplex sqrt.分母为0\t" << __LINE__ << std::endl;
            return res;
        }
        theta = atan2(c1.imag, c1.real);
        return ComplexStd::Complex(sqrt(r) * cos(0.5f * theta), sqrt(r) * sin(0.5f * theta));
    }

    ComplexStd::Complex SqrtComplex_unsafe(const ComplexStd::Complex& c1) {
        double r, theta;
        ComplexStd::Complex res(0, 0);
        r = sqrt(c1.real * c1.real + c1.imag * c1.imag);
        theta = atan2(c1.imag, c1.real);
        return ComplexStd::Complex(sqrt(r) * cos(0.5f * theta), sqrt(r) * sin(0.5f * theta));
    }


    ComplexStd::Complex MulComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2)
    {
        ComplexStd::Complex c3(0, 0);
        c3.real = c1.real * c2.real - c1.imag * c2.imag;
        c3.imag = c1.real * c2.imag + c1.imag * c2.real;
        return c3;
    }


    ComplexStd::Complex ExpComplex(const ComplexStd::Complex& c) {
        double f1 = exp(c.real);
        double f2 = cos(c.imag);
        double f3 = sin(c.imag);
        return ComplexStd::Complex(f1 * f2, f1 * f3);
    }
    ComplexStd::Complex MulDoubleComplex(double f, const ComplexStd::Complex& c) {

        return ComplexStd::Complex(f * c.real, f * c.imag);
    }
    ComplexStd::Complex MulComplexDouble(const ComplexStd::Complex& c, double f) {
        return MulDoubleComplex(f, c);
    }

    ComplexStd::Complex SubComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2) {
        return ComplexStd::Complex(c1.real - c2.real, c1.imag - c2.imag);
    }

    ComplexStd::Complex DivComplexComplex_unsafe(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2) {
        double temp1 = c1.real * c2.real + c1.imag * c2.imag;
        double temp2 = c2.real * c1.imag - c2.imag * c1.real;
        double temp3 = c2.real * c2.real + c2.imag * c2.imag;
        return ComplexStd::Complex(temp1 / temp3, temp2 / temp3);
    }

    ComplexStd::Complex GetNIByEpsilonIAndMuI(const ComplexStd::Complex& permittivityC, const ComplexStd::Complex& permeabilityC) {
        auto temp1 = MulComplexComplex(permittivityC, permeabilityC);
        auto temp2 = ComplexStd::SqrtComplex_unsafe(temp1);
        return MulDoubleComplex(GlobalConstantStd::C, temp2);
    }

}