// ApplyTransmissionInteraction: Fresnel TE/TM coherent + Jones vector + dynamic material + medium attenuation (v4 C1 + v5 D6-A)

#include "ApplyTransmissionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <cmath>

namespace rt {

namespace {

Complex CalcEpsC(double epsR, double sigma, double freqHz) {
    double omega = 6.28318530717958647693 * freqHz;
    double imag = (omega > 0.0) ? sigma / (omega * kEpsilon0) : 0.0;
    return Complex(epsR, -imag);
}

Complex FresnelTE_T(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex cosI_c(cosI, 0.0);
    return (cosI_c + cosI_c) / (cosI_c + sqrtTerm);
}

// v5 D6-A修复: TM透射分子由 ε_c → √ε_c (D1审计#4 Critical)
// T_TM = 2·√ε_c·cosθ_i / (ε_c·cosθ_i + √(ε_c - sin²θ_i))
Complex FresnelTM_T(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex sqrtEpsC = Sqrt(epsC);
    Complex nCos(sqrtEpsC.re * cosI, sqrtEpsC.im * cosI);
    Complex e_cos(epsC.re * cosI, epsC.im * cosI);
    return (nCos + nCos) / (e_cos + sqrtTerm);
}

} // namespace

bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    if (!node.transmission_semantic_complete) return false;
    if (node.medium_in_id < 0 || node.medium_out_id < 0) return false;
    if (node.medium_in_id == node.medium_out_id) return false;
    if (!input.material_db || input.material_db->empty()) return false;
    if (!input.scene || node.face_id < 0 || node.face_id >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& face = input.scene->faces[node.face_id];
    Vec3 kInc = Normalize(node.direction);
    Vec3 n = Normalize(node.surface_normal);
    std::string matName = (node.entered_from_front_side) ? face.back_material_name : face.front_material_name;
    if (matName.empty()) matName = "Concrete";
    MaterialProps txProps = input.material_db->QueryByName(matName, field.frequency_hz);

    Complex epsC = CalcEpsC(txProps.epsilon_r, txProps.sigma, field.frequency_hz);
    double cosI = std::fabs(Dot(kInc, n));
    if (cosI < 1e-9) cosI = 1e-9;

    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) eTE = MakeVec3(1.0, 0.0, 0.0);
    Vec3 eTM = Normalize(Cross(eTE, kInc));

    Complex A_inc(field.amplitude_real, field.amplitude_imag);
    Complex pTE(Dot(field.polarization_vector, eTE), Dot(field.polarization_imag, eTE));
    Complex pTM(Dot(field.polarization_vector, eTM), Dot(field.polarization_imag, eTM));
    Complex A_TE_inc = A_inc * pTE;
    Complex A_TM_inc = A_inc * pTM;

    Complex tTE = FresnelTE_T(cosI, epsC);
    Complex tTM = FresnelTM_T(cosI, epsC);
    Complex A_TE_trans = tTE * A_TE_inc;
    Complex A_TM_trans = tTM * A_TM_inc;

    double power_trans = A_TE_trans.NormSq() + A_TM_trans.NormSq();
    Complex amp_trans = A_TE_trans + A_TM_trans;

    double tx = A_TE_trans.re * eTE.x + A_TM_trans.re * eTM.x;
    double ty = A_TE_trans.re * eTE.y + A_TM_trans.re * eTM.y;
    double tz = A_TE_trans.re * eTE.z + A_TM_trans.re * eTM.z;
    double ix = A_TE_trans.im * eTE.x + A_TM_trans.im * eTM.x;
    double iy = A_TE_trans.im * eTE.y + A_TM_trans.im * eTM.y;
    double iz = A_TE_trans.im * eTE.z + A_TM_trans.im * eTM.z;

    field.amplitude_real = amp_trans.re;
    field.amplitude_imag = amp_trans.im;
    field.power_linear = power_trans;
    double polLen = std::sqrt(tx*tx + ty*ty + tz*tz);
    if (polLen < 1e-12) {
        field.polarization_vector = field.polarization_vector; // 保持入射极化方向
    } else {
        field.polarization_vector = Normalize(MakeVec3(tx, ty, tz));
    }
    field.polarization_imag = MakeVec3(ix, iy, iz);

    field.current_medium_id = node.medium_out_id;
    field.last_transmission_medium_in_id = node.medium_in_id;
    field.last_transmission_medium_out_id = node.medium_out_id;
    field.transmission_semantic_consumed = true;

    double alpha = (field.frequency_hz > 0.0)
        ? (6.28318530717958647693 * field.frequency_hz / kC0) * std::sqrt(std::max(0.0, -epsC.im))
        : 0.0;
    field.current_attenuation_np_per_m = alpha;

    return true;
}

} // namespace rt
