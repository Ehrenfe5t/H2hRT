// ApplyReflectionInteraction: Fresnel TE/TM coherent polarization + dynamic material lookup (v4 C1)
// Limitation: polarization stored as real vector (linear approx). Jones vector upgrade deferred to v5.

#include "ApplyReflectionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include "../common/material/MaterialDatabase.h"
#include <cmath>

namespace rt {

namespace {

Complex CalcEpsC(double epsR, double sigma, double freqHz) {
    double omega = 6.28318530717958647693 * freqHz;
    double imag = (omega > 0.0) ? sigma / (omega * kEpsilon0) : 0.0;
    return Complex(epsR, -imag);
}

Complex FresnelTE(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex cosI_c(cosI, 0.0);
    return (cosI_c - sqrtTerm) / (cosI_c + sqrtTerm);
}

Complex FresnelTM(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex e_cos(epsC.re * cosI, epsC.im * cosI);
    return (e_cos - sqrtTerm) / (e_cos + sqrtTerm);
}

} // namespace

bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    if (!input.material_db || input.material_db->empty()) return false;
    if (!input.scene || node.face_id < 0 || node.face_id >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& face = input.scene->faces[node.face_id];
    Vec3 kInc = Normalize(node.direction);
    Vec3 n = Normalize(node.surface_normal);
    std::string matName = (Dot(kInc, n) < 0.0) ? face.front_material_name : face.back_material_name;
    if (matName.empty()) matName = "Concrete";
    MaterialProps props = input.material_db->QueryByName(matName, field.frequency_hz);

    Complex epsC = CalcEpsC(props.epsilon_r, props.sigma, field.frequency_hz);
    double cosI = std::fabs(Dot(kInc, n));
    if (cosI < 1e-9) cosI = 1e-9;

    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) eTE = MakeVec3(1.0, 0.0, 0.0);
    Vec3 eTM = Normalize(Cross(eTE, kInc));

    Complex A_inc(field.amplitude_real, field.amplitude_imag);
    double pTE = Dot(field.polarization_vector, eTE);
    double pTM = Dot(field.polarization_vector, eTM);
    Complex A_TE_inc = A_inc * Complex(pTE, 0.0);
    Complex A_TM_inc = A_inc * Complex(pTM, 0.0);

    Complex gammaTE = FresnelTE(cosI, epsC);
    Complex gammaTM = FresnelTM(cosI, epsC);
    Complex A_TE_ref = gammaTE * A_TE_inc;
    Complex A_TM_ref = gammaTM * A_TM_inc;

    double power_ref = A_TE_ref.NormSq() + A_TM_ref.NormSq();
    Complex amp_ref = A_TE_ref + A_TM_ref;

    double rx = A_TE_ref.Real() * eTE.x + A_TM_ref.Real() * eTM.x;
    double ry = A_TE_ref.Real() * eTE.y + A_TM_ref.Real() * eTM.y;
    double rz = A_TE_ref.Real() * eTE.z + A_TM_ref.Real() * eTM.z;

    field.amplitude_real = amp_ref.re;
    field.amplitude_imag = amp_ref.im;
    field.power_linear = power_ref;
    field.phase_rad += (gammaTE.Arg() + gammaTM.Arg()) * 0.5;
    field.polarization_vector = Normalize(MakeVec3(rx, ry, rz));
    return true;
}

} // namespace rt
