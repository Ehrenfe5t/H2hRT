
#include"HdQComplex.h"



namespace ComplexStd {
    

    Complex::Complex() {
        real = 0.0;
        imag = 0.0;
    }

    Complex::Complex(const Complex& c) {
        if (&c == NULL) {
            real = 0.0;
            imag = 0.0;
        }
        else {
            real = c.real;
            imag = c.imag;
        }

    }

    Complex::Complex(double r, double i) {
        real = r;
        imag = i;
    }

    Complex::~Complex() {
    }

    std::string Complex::ToString() {
        std::string res = "(";
        res.append(std::to_string(real));
        res.append(",");
        res.append(std::to_string(imag));
        res.append(")");
        return res;
    }



    

}