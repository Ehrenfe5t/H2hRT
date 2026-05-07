// ApplyTransmissionInteraction: Fresnel TE/TM coherent polarization + dynamic material + medium attenuation (v4 C1)

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

Complex FresnelTM_T(double cosI, const Complex& epsC) {
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex e_cos(epsC.re * cosI, epsC.im * cosI);
    return (e_cos + e_cos) / (e_cos + sqrtTerm);
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
    double pTE = Dot(field.polarization_vector, eTE);
    double pTM = Dot(field.polarization_vector, eTM);
    Complex A_TE_inc = A_inc * Complex(pTE, 0.0);
    Complex A_TM_inc = A_inc * Complex(pTM, 0.0);

    Complex tTE = FresnelTE_T(cosI, epsC);
    Complex tTM = FresnelTM_T(cosI, epsC);
    Complex A_TE_trans = tTE * A_TE_inc;
    Complex A_TM_trans = tTM * A_TM_inc;

    double power_trans = A_TE_trans.NormSq() + A_TM_trans.NormSq();
    Complex amp_trans = A_TE_trans + A_TM_trans;

    double tx = A_TE_trans.Real() * eTE.x + A_TM_trans.Real() * eTM.x;
    double ty = A_TE_trans.Real() * eTE.y + A_TM_trans.Real() * eTM.y;
    double tz = A_TE_trans.Real() * eTE.z + A_TM_trans.Real() * eTM.z;

    field.amplitude_real = amp_trans.re;
    field.amplitude_imag = amp_trans.im;
    field.power_linear = power_trans;
    field.phase_rad += (tTE.Arg() + tTM.Arg()) * 0.5;
    field.polarization_vector = Normalize(MakeVec3(tx, ty, tz));

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
