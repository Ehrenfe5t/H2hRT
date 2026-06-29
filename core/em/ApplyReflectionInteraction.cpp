// ApplyReflectionInteraction: Fresnel TE/TM coherent polarization + Jones vector (v4 C1 + v5 D6-A)
// v5: 复极化投影/重构, 替换实向量近似, 支持椭圆极化传播

#include "ApplyReflectionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/ComplexVec3.h"
#include "../common/math/MathConstants.h"
#include "../common/material/MaterialDatabase.h"
#include "FresnelInterface.h"
#include <cmath>

namespace rt {

bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    if (!input.scene || node.face_id < 0 || node.face_id >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& face = input.scene->faces[node.face_id];
    Vec3 kOut = Normalize(node.direction);
    Vec3 n = Normalize(node.surface_normal);
    Vec3 kInc = Length(node.incident_direction) > 0.5
        ? Normalize(node.incident_direction) : Normalize(Reflect(kOut, n));
    double cosI = std::fabs(Dot(kInc, n));
    const bool fromFront = Dot(kInc, n) < 0.0;
    MaterialProps incidentProps, transmittedProps;
    if (input.material_db && !input.material_db->empty()) {
        const std::string& incidentName = fromFront ? face.front_material_name : face.back_material_name;
        const std::string& transmittedName = fromFront ? face.back_material_name : face.front_material_name;
        if (!incidentName.empty()) incidentProps = input.material_db->QueryByName(incidentName, field.frequency_hz);
        if (!transmittedName.empty()) transmittedProps = input.material_db->QueryByName(transmittedName, field.frequency_hz);
    }
    const FresnelInterfaceCoefficients fresnel = EvaluateFresnelInterface(
        incidentProps, transmittedProps, cosI, field.frequency_hz);
    if (cosI < 1e-9) cosI = 1e-9;

    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) {
        if (std::fabs(kInc.x) < 0.9) eTE = Normalize(Cross(kInc, MakeVec3(1.0, 0.0, 0.0)));
        else                       eTE = Normalize(Cross(kInc, MakeVec3(0.0, 1.0, 0.0)));
    }
    Vec3 eTMIn = Normalize(Cross(eTE, kInc));
    Vec3 eTMOut = Normalize(Cross(eTE, kOut));

    Complex A_inc(field.amplitude_real, field.amplitude_imag);
    // v5 Jones: 复投影到TE/TM基
    Complex pTE(Dot(field.polarization_vector, eTE), Dot(field.polarization_imag, eTE));
    Complex pTM(Dot(field.polarization_vector, eTMIn), Dot(field.polarization_imag, eTMIn));
    Complex A_TE_inc = A_inc * pTE;
    Complex A_TM_inc = A_inc * pTM;

    Complex gammaTE = fresnel.reflection_te;
    Complex gammaTM = fresnel.reflection_tm;

    // ── v9 B2-a: 复矢量路径 ──
    if (field.vector_field_valid) {
        // 投影: E_in → TE, TM 复分量
        Complex E_TE = ComplexDot(field.electric_field_world, eTE);
        Complex E_TM = ComplexDot(field.electric_field_world, eTMIn);

        // Fresnel: 各分量乘复系数
        Complex E_TE_ref = gammaTE * E_TE;
        Complex E_TM_ref = gammaTM * E_TM;

        // 重构: 从TE/TM基重新构建世界复电场
        field.electric_field_world = ReconstructFromBasis(E_TE_ref, eTE, E_TM_ref, eTMOut);

        field.SyncLegacyFields();
        return true;
    }

    // ── 旧标量路径 (兼容) ──
    Complex A_TE_ref = gammaTE * A_TE_inc;
    Complex A_TM_ref = gammaTM * A_TM_inc;

    // v7.4 B11: 交互相位累加到 phase_rad (能量加权平均)
    double teW=A_TE_ref.NormSq(), tmW=A_TM_ref.NormSq(), totalW=teW+tmW;
    if (totalW > 1e-30) field.phase_rad += std::atan2(
        A_TE_ref.im*teW + A_TM_ref.im*tmW, A_TE_ref.re*teW + A_TM_ref.re*tmW);

    double power_ref = totalW;
    // v7 C5修复: TE/TM正交分量不做标量加法，复振幅由Jones矢量编码
    // amplitude存储|E|，Jones矢量(实部+虚部)存储归一化复方向
    double ampMag = std::sqrt(std::max(0.0, power_ref));

    // v5 Jones: 全复极化重构 — 在世界坐标中构建复场矢量并归一化
    double rx = A_TE_ref.re * eTE.x + A_TM_ref.re * eTMOut.x;
    double ry = A_TE_ref.re * eTE.y + A_TM_ref.re * eTMOut.y;
    double rz = A_TE_ref.re * eTE.z + A_TM_ref.re * eTMOut.z;
    double ix = A_TE_ref.im * eTE.x + A_TM_ref.im * eTMOut.x;
    double iy = A_TE_ref.im * eTE.y + A_TM_ref.im * eTMOut.y;
    double iz = A_TE_ref.im * eTE.z + A_TM_ref.im * eTMOut.z;

    field.amplitude_real = ampMag;
    field.amplitude_imag = 0.0;
    field.power_linear = power_ref;
    // 极化归一化: 实部+虚部联合归一化使|Jones|=1
    double fullPolLen = std::sqrt(rx*rx + ry*ry + rz*rz + ix*ix + iy*iy + iz*iz);
    if (fullPolLen > 1e-12) {
        double pInv = 1.0 / fullPolLen;
        field.polarization_vector = MakeVec3(rx * pInv, ry * pInv, rz * pInv);
        field.polarization_imag    = MakeVec3(ix * pInv, iy * pInv, iz * pInv);
    } else {
        // 退化为零场: 保持入射极化方向
    }
    return true;
}

} // namespace rt
