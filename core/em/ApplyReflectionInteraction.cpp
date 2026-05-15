// ApplyReflectionInteraction: Fresnel TE/TM coherent polarization + Jones vector (v4 C1 + v5 D6-A)
// v5: 复极化投影/重构, 替换实向量近似, 支持椭圆极化传播

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
    if (!input.scene || node.face_id < 0 || node.face_id >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& face = input.scene->faces[node.face_id];
    Vec3 kOut = Normalize(node.direction);
    Vec3 n = Normalize(node.surface_normal);
    Vec3 kInc = Reflect(kOut, n); // v7.4 B10: 从出射恢复入射方向, eTM需用kInc
    // v7.4 几何-EM解耦: 材质不可用→真空(ε_r=1,|Γ|=0), EM自然淘汰而非return false
    double cosI = std::fabs(Dot(kOut, n));
    MaterialProps props; // 默认真空
    if (input.material_db && !input.material_db->empty() && !face.surface_material_name.empty())
        props = input.material_db->QueryByName(face.surface_material_name, field.frequency_hz);
    Complex epsC = CalcEpsC(props.epsilon_r, props.sigma, field.frequency_hz);
    if (cosI < 1e-9) cosI = 1e-9;

    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) {
        if (std::fabs(kInc.x) < 0.9) eTE = Normalize(Cross(kInc, MakeVec3(1.0, 0.0, 0.0)));
        else                       eTE = Normalize(Cross(kInc, MakeVec3(0.0, 1.0, 0.0)));
    }
    Vec3 eTM = Normalize(Cross(eTE, kInc));

    Complex A_inc(field.amplitude_real, field.amplitude_imag);
    // v5 Jones: 复投影到TE/TM基
    Complex pTE(Dot(field.polarization_vector, eTE), Dot(field.polarization_imag, eTE));
    Complex pTM(Dot(field.polarization_vector, eTM), Dot(field.polarization_imag, eTM));
    Complex A_TE_inc = A_inc * pTE;
    Complex A_TM_inc = A_inc * pTM;

    Complex gammaTE = FresnelTE(cosI, epsC);
    Complex gammaTM = FresnelTM(cosI, epsC);
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
    double rx = A_TE_ref.re * eTE.x + A_TM_ref.re * eTM.x;
    double ry = A_TE_ref.re * eTE.y + A_TM_ref.re * eTM.y;
    double rz = A_TE_ref.re * eTE.z + A_TM_ref.re * eTM.z;
    double ix = A_TE_ref.im * eTE.x + A_TM_ref.im * eTM.x;
    double iy = A_TE_ref.im * eTE.y + A_TM_ref.im * eTM.y;
    double iz = A_TE_ref.im * eTE.z + A_TM_ref.im * eTM.z;

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
