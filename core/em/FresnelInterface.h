#pragma once

#include "../common/material/MaterialDatabase.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"

#include <algorithm>

namespace rt {

struct FresnelInterfaceCoefficients {
    Complex n1;
    Complex n2;
    Complex cos_t;
    Complex reflection_te;
    Complex reflection_tm;
    Complex transmission_te;
    Complex transmission_tm;
};

inline Complex MaterialComplexPermittivity(const MaterialProps& props, double frequencyHz) {
    const double omega = kTwoPi * frequencyHz;
    const double loss = omega > 0.0 ? props.sigma / (omega * kEpsilon0) : 0.0;
    return Complex(std::max(1.0, props.epsilon_r), -loss);
}

inline FresnelInterfaceCoefficients EvaluateFresnelInterface(
    const MaterialProps& incident,
    const MaterialProps& transmitted,
    double cosIncident,
    double frequencyHz)
{
    FresnelInterfaceCoefficients result;
    result.n1 = Sqrt(MaterialComplexPermittivity(incident, frequencyHz));
    result.n2 = Sqrt(MaterialComplexPermittivity(transmitted, frequencyHz));
    const double cosI = std::max(1.0e-12, std::min(1.0, cosIncident));
    const double sin2I = std::max(0.0, 1.0 - cosI * cosI);
    const Complex ratio = result.n1 / result.n2;
    result.cos_t = Sqrt(Complex(1.0, 0.0) - ratio * ratio * Complex(sin2I, 0.0));
    if (result.cos_t.re < 0.0) result.cos_t = result.cos_t * -1.0;

    const Complex n1CosI = result.n1 * cosI;
    const Complex n2CosI = result.n2 * cosI;
    const Complex n1CosT = result.n1 * result.cos_t;
    const Complex n2CosT = result.n2 * result.cos_t;
    result.reflection_te = (n1CosI - n2CosT) / (n1CosI + n2CosT);
    result.reflection_tm = (n2CosI - n1CosT) / (n2CosI + n1CosT);
    result.transmission_te = (n1CosI * 2.0) / (n1CosI + n2CosT);
    result.transmission_tm = (n1CosI * 2.0) / (n2CosI + n1CosT);
    return result;
}

} // namespace rt
