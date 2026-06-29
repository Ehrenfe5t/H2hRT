// ApplyTransmissionInteraction: Fresnel TE/TM coherent + Jones vector + dynamic material + medium attenuation (v4 C1 + v5 D6-A)

#include "ApplyTransmissionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/ComplexVec3.h"
#include "../common/math/MathConstants.h"
#include "FresnelInterface.h"
#include <cmath>

namespace rt {

bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    if (!node.transmission_semantic_complete) return false;
    if (node.medium_in_id < 0 || node.medium_out_id < 0) return false;
    if (node.medium_in_id == node.medium_out_id) return false;
    if (!input.scene || node.face_id < 0 || node.face_id >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& face = input.scene->faces[node.face_id];
    Vec3 n = Normalize(node.surface_normal);
    Vec3 kInc = (Length(node.incident_direction) > 0.0)
        ? Normalize(node.incident_direction) : MakeVec3(-node.direction.x,-node.direction.y,-node.direction.z);
    Vec3 kOut = Normalize(node.direction);
    const bool fromFront = Dot(kInc, n) < 0.0;
    MaterialProps incidentProps, transmittedProps;
    if (input.material_db && !input.material_db->empty()) {
        const std::string& incidentName = fromFront ? face.front_material_name : face.back_material_name;
        const std::string& transmittedName = fromFront ? face.back_material_name : face.front_material_name;
        if (!incidentName.empty()) incidentProps = input.material_db->QueryByName(incidentName, field.frequency_hz);
        if (!transmittedName.empty()) transmittedProps = input.material_db->QueryByName(transmittedName, field.frequency_hz);
    }

    double cosI = std::fabs(Dot(kInc, n));
    if (cosI < 1e-9) cosI = 1e-9;
    const FresnelInterfaceCoefficients fresnel = EvaluateFresnelInterface(
        incidentProps, transmittedProps, cosI, field.frequency_hz);
    // Convert electric-field transmission coefficients to power-normalized
    // channel-wave coefficients. This makes R+T=1 for a lossless interface.
    const double incidentFlux = std::max(1.0e-15, fresnel.n1.re * cosI);
    const double transmittedFlux = std::max(0.0, (fresnel.n2 * fresnel.cos_t).re);
    const double fluxScale = std::sqrt(transmittedFlux / incidentFlux);

    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) {
        // 正常入射: Cross(kInc,n)退化, 用任意与kInc垂直的方向
        if (std::fabs(kInc.x) < 0.9) eTE = Normalize(Cross(kInc, MakeVec3(1.0, 0.0, 0.0)));
        else                       eTE = Normalize(Cross(kInc, MakeVec3(0.0, 1.0, 0.0)));
    }
    Vec3 eTMIn = Normalize(Cross(eTE, kInc));
    Vec3 eTMOut = Normalize(Cross(eTE, kOut));

    Complex A_inc(field.amplitude_real, field.amplitude_imag);
    Complex pTE(Dot(field.polarization_vector, eTE), Dot(field.polarization_imag, eTE));
    Complex pTM(Dot(field.polarization_vector, eTMIn), Dot(field.polarization_imag, eTMIn));
    Complex A_TE_inc = A_inc * pTE;
    Complex A_TM_inc = A_inc * pTM;

    Complex tTE = fresnel.transmission_te * fluxScale;
    Complex tTM = fresnel.transmission_tm * fluxScale;

    // ── v9 B2-b: 复矢量路径 ──
    if (field.vector_field_valid) {
        Complex E_TE = ComplexDot(field.electric_field_world, eTE);
        Complex E_TM = ComplexDot(field.electric_field_world, eTMIn);

        Complex E_TE_trans = tTE * E_TE;
        Complex E_TM_trans = tTM * E_TM;

        field.electric_field_world = ReconstructFromBasis(E_TE_trans, eTE, E_TM_trans, eTMOut);

        field.current_medium_id = node.medium_out_id;
        field.last_transmission_medium_in_id = node.medium_in_id;
        field.last_transmission_medium_out_id = node.medium_out_id;
        field.transmission_semantic_consumed = true;

        double alpha = (field.frequency_hz > 0.0)
            ? (kTwoPi * field.frequency_hz / kC0) * std::fabs(fresnel.n2.im)
            : 0.0;
        field.current_attenuation_np_per_m = alpha;
        field.current_refractive_index = std::max(1.0, fresnel.n2.re);

        field.SyncLegacyFields();
        return true;
    }

    // ── 旧标量路径 (兼容) ──
    Complex A_TE_trans = tTE * A_TE_inc;
    Complex A_TM_trans = tTM * A_TM_inc;

    // v7.4 B16: 交互相位累加
    double teW=A_TE_trans.NormSq(), tmW=A_TM_trans.NormSq(), totalW=teW+tmW;
    if (totalW > 1e-30) field.phase_rad += std::atan2(
        A_TE_trans.im*teW + A_TM_trans.im*tmW, A_TE_trans.re*teW + A_TM_trans.re*tmW);

    double power_trans = totalW;
    // v7 C5修复: TE/TM正交分量不做标量加法，复振幅由Jones矢量编码
    double ampMag = std::sqrt(std::max(0.0, power_trans));

    double tx = A_TE_trans.re * eTE.x + A_TM_trans.re * eTMOut.x;
    double ty = A_TE_trans.re * eTE.y + A_TM_trans.re * eTMOut.y;
    double tz = A_TE_trans.re * eTE.z + A_TM_trans.re * eTMOut.z;
    double ix = A_TE_trans.im * eTE.x + A_TM_trans.im * eTMOut.x;
    double iy = A_TE_trans.im * eTE.y + A_TM_trans.im * eTMOut.y;
    double iz = A_TE_trans.im * eTE.z + A_TM_trans.im * eTMOut.z;

    field.amplitude_real = ampMag;
    field.amplitude_imag = 0.0;
    field.power_linear = power_trans;
    double fullPolLen = std::sqrt(tx*tx + ty*ty + tz*tz + ix*ix + iy*iy + iz*iz);
    if (fullPolLen > 1e-12) {
        double pInv = 1.0 / fullPolLen;
        field.polarization_vector = MakeVec3(tx * pInv, ty * pInv, tz * pInv);
        field.polarization_imag    = MakeVec3(ix * pInv, iy * pInv, iz * pInv);
    }

    field.current_medium_id = node.medium_out_id;
    field.last_transmission_medium_in_id = node.medium_in_id;
    field.last_transmission_medium_out_id = node.medium_out_id;
    field.transmission_semantic_consumed = true;

    double alpha = (field.frequency_hz > 0.0)
        ? (kTwoPi * field.frequency_hz / kC0) * std::fabs(fresnel.n2.im)
        : 0.0;
    field.current_attenuation_np_per_m = alpha;
    // v7 H2: 透射后介质折射率, 用于后续自由空间段相位和时延修正
    field.current_refractive_index = std::max(1.0, fresnel.n2.re);

    return true;
}

} // namespace rt
