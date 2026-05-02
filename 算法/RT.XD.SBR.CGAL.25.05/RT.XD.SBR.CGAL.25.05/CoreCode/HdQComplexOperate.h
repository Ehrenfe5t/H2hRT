#pragma once

#include"HdQComplex.h"

namespace ComplexStd {


    double Length(const Complex& c);

    /// <summary>
    /// ¸³Öµ²Ù×÷ 
    /// </summary>
    /// <param name="e1"></param>
    /// <param name="e2"></param>
    void AssignmentComplexComplex(ComplexStd::Complex& e1, const ComplexStd::Complex& e2);

    ComplexStd::Complex AddComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2);

    ComplexStd::Complex AddComplexComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2, const ComplexStd::Complex& c3);

    ComplexStd::Complex SqrtComplex(const ComplexStd::Complex& c1);

    ComplexStd::Complex SqrtComplex_unsafe(const ComplexStd::Complex& c1);



    ComplexStd::Complex MulComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2);



    ComplexStd::Complex ExpComplex(const ComplexStd::Complex& c);

    ComplexStd::Complex MulDoubleComplex(double f, const ComplexStd::Complex& c);


    ComplexStd::Complex MulComplexDouble(const ComplexStd::Complex& c, double f);


    ComplexStd::Complex SubComplexComplex(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2);

    ComplexStd::Complex DivComplexComplex_unsafe(const ComplexStd::Complex& c1, const ComplexStd::Complex& c2);

    ComplexStd::Complex GetNIByEpsilonIAndMuI(const ComplexStd::Complex& permittivityC, const ComplexStd::Complex& permeabilityC);

}