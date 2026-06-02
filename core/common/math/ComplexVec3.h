// 文件目标：
// - v9 主线B B1-a: 定义三维复矢量类型及基础运算。
//
// 主要功能：
// - ComplexVec3 作为复电场矢量的基础类型
// - 支持加减、复标量乘、模平方、归一化、实基投影、复内积
// - TE/TM或soft/hard基上的投影与重构

#pragma once

#include "Complex.h"
#include "Vec3.h"

#include <cmath>

namespace rt {

/// <summary>
/// 三维复矢量, 用于表达世界坐标中的复电场 E = [Ex, Ey, Ez]。
/// </summary>
struct ComplexVec3 {
    Complex x;
    Complex y;
    Complex z;

    ComplexVec3() = default;
    ComplexVec3(const Complex& cx, const Complex& cy, const Complex& cz) : x(cx), y(cy), z(cz) {}
};

// ── 基础运算 ──

/// <summary>复矢量加法。</summary>
inline ComplexVec3 Add(const ComplexVec3& a, const ComplexVec3& b) {
    return ComplexVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/// <summary>复矢量减法。</summary>
inline ComplexVec3 Subtract(const ComplexVec3& a, const ComplexVec3& b) {
    return ComplexVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/// <summary>复标量乘以实方向 — 生成复矢量 (用于从基矢量构造复场)。</summary>
inline ComplexVec3 ScaleComplexVec3(const Vec3& v, const Complex& c) {
    return ComplexVec3(
        Complex(v.x * c.re, v.x * c.im),
        Complex(v.y * c.re, v.y * c.im),
        Complex(v.z * c.re, v.z * c.im));
}

/// <summary>复标量乘以复矢量。</summary>
inline ComplexVec3 Scale(const ComplexVec3& v, const Complex& c) {
    return ComplexVec3(v.x * c, v.y * c, v.z * c);
}

/// <summary>实数缩放复矢量。</summary>
inline ComplexVec3 Scale(const ComplexVec3& v, double s) {
    return ComplexVec3(v.x * s, v.y * s, v.z * s);
}

// ── 度量 ──

/// <summary>复矢量模平方: |Ex|² + |Ey|² + |Ez|²。</summary>
inline double NormSq(const ComplexVec3& v) {
    return v.x.NormSq() + v.y.NormSq() + v.z.NormSq();
}

/// <summary>复矢量模: sqrt(|Ex|² + |Ey|² + |Ez|²)。</summary>
inline double Norm(const ComplexVec3& v) {
    return std::sqrt(NormSq(v));
}

/// <summary>归一化复矢量 — 保持各分量之间的复比例关系。</summary>
inline ComplexVec3 Normalize(const ComplexVec3& v) {
    double n = Norm(v);
    if (n <= 1e-15) return ComplexVec3(); // 零场
    double inv = 1.0 / n;
    return ComplexVec3(v.x * inv, v.y * inv, v.z * inv);
}

// ── 投影与重构 (用于TE/TM, soft/hard, theta/phi基变换) ──

/// <summary>
/// 复矢量到实单位基矢量的复投影 (复内积)。
/// 返回复标量 c = E · v  (其中v是实单位矢量)。
/// 注意: 这不是Hermitian内积, 而是点积 — 用于将电场投影到局部极化基。
/// </summary>
inline Complex ComplexDot(const ComplexVec3& E, const Vec3& v) {
    return Complex(
        E.x.re * v.x + E.y.re * v.y + E.z.re * v.z,
        E.x.im * v.x + E.y.im * v.y + E.z.im * v.z);
}

/// <summary>
/// 两个复矢量之间的复点积 (分量对分量相乘后求和)。
/// E1 · E2 = E1_x*E2_x + E1_y*E2_y + E1_z*E2_z
/// </summary>
inline Complex ComplexDot(const ComplexVec3& a, const ComplexVec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// <summary>
/// 从两个正交实基矢量上的复分量重构世界坐标复矢量。
/// E_world = c1 * basis1 + c2 * basis2
/// 反射/透射: basis1 = eTE, basis2 = eTM
/// 绕射: basis1 = e_soft, basis2 = e_hard
/// 天线: basis1 = e_theta, basis2 = e_phi
/// </summary>
inline ComplexVec3 ReconstructFromBasis(
    const Complex& c1, const Vec3& basis1,
    const Complex& c2, const Vec3& basis2)
{
    return ComplexVec3(
        Complex(c1.re * basis1.x + c2.re * basis2.x, c1.im * basis1.x + c2.im * basis2.x),
        Complex(c1.re * basis1.y + c2.re * basis2.y, c1.im * basis1.y + c2.im * basis2.y),
        Complex(c1.re * basis1.z + c2.re * basis2.z, c1.im * basis1.z + c2.im * basis2.z));
}

} // namespace rt
