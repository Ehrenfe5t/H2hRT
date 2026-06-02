// 文件目标：
// - 定义三维向量基本类型及常用向量运算。
//
// 主要功能：
// - 提供 Vec3 / Point3 基础类型；
// - 实现加减、缩放、点积、叉积、归一化、反射、折射等常用向量操作；
// - 作为整个系统几何计算的基础数学层。

#pragma once

#include <cmath>

#include "MathConstants.h"

namespace rt {

/// <summary>
/// 三维向量结构，用作方向和位置的基础类型。
/// </summary>
struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

typedef Vec3 Point3;

/// <summary>
/// v9 步骤1: Snell折射的结构化结果，替代旧SnellRefract的零向量返回。
/// 包含完整诊断信息：入射角、出射角、TIR标志、Snell残差。
/// </summary>
struct SnellResult {
    bool valid = false;                     // 计算是否有效
    bool total_internal_reflection = false; // 是否发生全内反射
    Vec3 direction;                         // 折射方向 (TIR时为零向量)
    double cos_i = 0.0;                     // |cos(入射角)|, 已clamp到[0,1]
    double cos_t = 0.0;                     // |cos(出射角)|, TIR时为0
    double theta_i_rad = 0.0;               // 入射角 (弧度)
    double theta_t_rad = 0.0;               // 出射角 (弧度), TIR时为pi/2
    double residual = 0.0;                  // |n1*sin(theta_i) - n2*sin(theta_t)|
};

// v8 GPU: nvcc EDG front-end cannot parse MSVC-stdlib-dependent inline functions.
// When included from .cu files, only struct definitions are visible.
// .cu files do not call these functions (query logic is in .cpp files).
#ifndef __CUDACC__

/// <summary>
/// 构造一个方向向量。
/// </summary>
/// <param name="x">X 分量。</param>
/// <param name="y">Y 分量。</param>
/// <param name="z">Z 分量。</param>
/// <returns>构造完成的 Vec3。</returns>
inline Vec3 MakeVec3(double x, double y, double z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

/// <summary>
/// 向量减法 (a - b)。
/// </summary>
/// <param name="a">被减向量。</param>
/// <param name="b">减向量。</param>
/// <returns>a - b 的结果向量。</returns>
inline Vec3 SubtractVec(const Vec3& a, const Vec3& b) {
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/// <summary>
/// 点减法，得到从 b 指向 a 的向量。
/// </summary>
/// <param name="a">终点。</param>
/// <param name="b">起点。</param>
/// <returns>a - b 的方向向量。</returns>
inline Vec3 Subtract(const Point3& a, const Point3& b) {
    return MakeVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/// <summary>
/// 向量加法。
/// </summary>
/// <param name="a">左向量。</param>
/// <param name="b">右向量。</param>
/// <returns>a + b 的和向量。</returns>
inline Vec3 AddVec(const Vec3& a, const Vec3& b) {
    return MakeVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/// <summary>
/// 点加向量，沿向量方向平移点。
/// </summary>
/// <param name="a">原始点。</param>
/// <param name="b">位移向量。</param>
/// <returns>平移后的新点。</returns>
inline Point3 Add(const Point3& a, const Vec3& b) {
    Point3 r; r.x = a.x + b.x; r.y = a.y + b.y; r.z = a.z + b.z; return r;
}

/// <summary>
/// 向量标量乘法。
/// </summary>
/// <param name="v">原始向量。</param>
/// <param name="s">标量因子。</param>
/// <returns>缩放后的向量。</returns>
inline Vec3 Scale(const Vec3& v, double s) {
    return MakeVec3(v.x * s, v.y * s, v.z * s);
}

/// <summary>
/// 向量点积（内积）。
/// </summary>
/// <param name="a">左向量。</param>
/// <param name="b">右向量。</param>
/// <returns>点积标量值。</returns>
inline double Dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// <summary>
/// 向量叉积（外积）。
/// </summary>
/// <param name="a">左向量。</param>
/// <param name="b">右向量。</param>
/// <returns>a x b 的正交向量。</returns>
inline Vec3 Cross(const Vec3& a, const Vec3& b) {
    return MakeVec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

/// <summary>
/// 向量长度的平方（比 Length 快，避免开方）。
/// </summary>
/// <param name="v">输入向量。</param>
/// <returns>|v|^2。</returns>
inline double LengthSq(const Vec3& v) {
    return Dot(v, v);
}

/// <summary>
/// 向量欧几里得长度。
/// </summary>
/// <param name="v">输入向量。</param>
/// <returns>|v|。</returns>
inline double Length(const Vec3& v) {
    return std::sqrt(LengthSq(v));
}

/// <summary>
/// 将向量归一化为单位长度。
/// </summary>
/// <param name="v">输入向量。</param>
/// <returns>单位向量；零向量输入返回零向量。</returns>
inline Vec3 Normalize(const Vec3& v) {
    double len = Length(v);
    if (len <= 0.0) { return Vec3{}; }
    return Scale(v, 1.0 / len);
}
// v6 C4: 安全归一化, 零向量时返回fallback而非零向量
inline Vec3 SafeNormalize(const Vec3& v, const Vec3& fallback = MakeVec3(1,0,0)) {
    double len = Length(v);
    if (len <= 1e-12) { return fallback; }
    return Scale(v, 1.0 / len);
}

/// <summary>
/// 计算方向向量关于法线的镜面反射方向。
/// </summary>
/// <param name="dir">入射方向。</param>
/// <param name="normal">表面法线（将被归一化）。</param>
/// <returns>反射方向向量。</returns>
inline Vec3 Reflect(const Vec3& dir, const Vec3& normal) {
    Vec3 n = Normalize(normal);
    double d = Dot(dir, n);
    return MakeVec3(
        dir.x - 2.0 * d * n.x,
        dir.y - 2.0 * d * n.y,
        dir.z - 2.0 * d * n.z);
}

/// <summary>
/// 将值钳制到 [lo, hi] 区间。
/// </summary>
/// <param name="value">输入值。</param>
/// <param name="lo">下限。</param>
/// <param name="hi">上限。</param>
/// <returns>钳制后的值。</returns>
/// <summary>
/// 关于平面做镜像: 将 point 关于过 planePoint、法向 planeNormal 的平面做镜像。
/// </summary>
inline Point3 MirrorPointAcrossPlane(const Point3& point, const Point3& planePoint, const Vec3& planeNormal) {
    const Vec3 delta = Subtract(point, planePoint);
    const double signedDistance = Dot(delta, planeNormal);
    return Add(point, Scale(planeNormal, -2.0 * signedDistance));
}

/// <summary>
/// 将值钳制到 [lo, hi] 区间。
inline double Clamp(double value, double lo, double hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

/// <summary>
/// 计算折射方向（斯涅尔定律）。
/// </summary>
/// <param name="incident">入射方向。</param>
/// <param name="normal">表面法线。</param>
/// <param name="n1">入射介质折射率。</param>
/// <param name="n2">透射介质折射率。</param>
/// <returns>折射方向；若发生全内反射则返回零向量。</returns>
inline Vec3 SnellRefract(const Vec3& incident, const Vec3& normal, double n1, double n2) {
    double eta = n1 / n2;
    double cosI = -Dot(incident, normal);
    double sinT2 = eta * eta * (1.0 - cosI * cosI);
    if (sinT2 > 1.0) {
        return Vec3{};  // 全内反射，返回零向量
    }
    double cosT = std::sqrt(1.0 - sinT2);
    return MakeVec3(
        eta * incident.x + (eta * cosI - cosT) * normal.x,
        eta * incident.y + (eta * cosI - cosT) * normal.y,
        eta * incident.z + (eta * cosI - cosT) * normal.z);
}

/// <summary>
/// v9 增强版Snell折射计算，返回结构化SnellResult。
/// 相比旧版SnellRefract的改进：
///   - 自动翻转法线，确保cos_i>=0（背面入射时自动校正）
///   - cosI clamp到[0,1]，防止浮点误差导致NaN
///   - 输出入射角/出射角/TIR标志/Snell残差等诊断信息
/// </summary>
/// <param name="incident">入射方向（指向表面）。</param>
/// <param name="normal">表面法线（任意朝向，内部自动校正）。</param>
/// <param name="n1">入射侧介质折射率。</param>
/// <param name="n2">出射侧介质折射率。</param>
/// <returns>结构化SnellResult，含折射方向和完整诊断。</returns>
inline SnellResult SnellRefractV2(const Vec3& incident, const Vec3& normal, double n1, double n2) {
    SnellResult result;

    // 自动翻转法线: 确保法线指向入射侧 (Dot(incident, effectiveNormal) <= 0)
    Vec3 effectiveNormal = normal;
    double dotIN = Dot(incident, normal);
    if (dotIN > 0.0) {
        // 入射方向与法线同侧 → 法线需要翻转
        effectiveNormal = Scale(normal, -1.0);
        dotIN = -dotIN;
    }

    // cos(入射角) = -Dot(incident, effectiveNormal), 应 >= 0
    double cosI = -dotIN;
    // Clamp到[0,1]防止浮点误差导致cosI>1或cosI<0
    if (cosI > 1.0) cosI = 1.0;
    if (cosI < 0.0) cosI = 0.0;

    result.cos_i = cosI;
    result.theta_i_rad = std::acos(cosI);

    double eta = n1 / n2;
    double sin2I = 1.0 - cosI * cosI;
    double sinT2 = eta * eta * sin2I;

    if (sinT2 >= 1.0) {
        // ── 全内反射 ──
        result.total_internal_reflection = true;
        result.valid = true;
        result.cos_t = 0.0;
        result.theta_t_rad = kHalfPi;
        // TIR物理上仍在临界角满足Snell: n1*sin(theta_c) = n2*sin(pi/2)
        result.residual = 0.0;
        // direction保持零向量
        return result;
    }

    double cosT = std::sqrt(1.0 - sinT2);
    result.cos_t = cosT;
    result.theta_t_rad = std::acos(cosT);

    // Snell残差: |n1*sin(theta_i) - n2*sin(theta_t)|
    double sinI = std::sqrt(sin2I);
    double sinT = std::sqrt(1.0 - cosT * cosT);
    result.residual = std::fabs(n1 * sinI - n2 * sinT);

    // 计算折射方向: eta * incident + (eta*cos_i - cos_t) * effectiveNormal
    double coeff = eta * cosI - cosT;
    result.direction = MakeVec3(
        eta * incident.x + coeff * effectiveNormal.x,
        eta * incident.y + coeff * effectiveNormal.y,
        eta * incident.z + coeff * effectiveNormal.z);

    result.valid = true;
    return result;
}

#endif // __CUDACC__

} // namespace rt
