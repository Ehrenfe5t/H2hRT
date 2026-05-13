// ApplyDiffractionInteraction: UTD edge diffraction + Jones vector (v4 C3 + v5 D6-A)
// C3-A: phi' computed from incident_direction | C3-B: s2 from path geometry
// C3-C: Fresnel F(x) via 8-point Gauss-Legendre | v5: 复极化soft/hard投影

#include "ApplyDiffractionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <cmath>
#include <cstdlib>

namespace rt {

namespace {

// ---- C3-C: Numerical Fresnel transition function ----
// F(x) = 2*j*sqrt(x)*exp(j*x) * integral_{sqrt(x)}^{inf} exp(-j*tau^2) dtau
// Numerical integration via 8-point Gauss-Legendre quadrature with adaptive interval refinement

Complex FresnelIntegralTailNumerical(double tau0, int NIntervals = 16)
{
    // GL 8-point weights and abscissae on [-1, 1]
    static const double glX[8] = {-0.960289856497536, -0.796666477413627, -0.525532409916329, -0.183434642495650,
                                    0.183434642495650,  0.525532409916329,  0.796666477413627,  0.960289856497536};
    static const double glW[8] = { 0.101228536290376,  0.222381034453374,  0.313706645877887,  0.362683783378362,
                                    0.362683783378362,  0.313706645877887,  0.222381034453374,  0.101228536290376};

    // Adaptive: integrate over [tau0, tau0 + NIntervals * step] where step = 0.5
    double step = 0.5;
    Complex sum(0.0, 0.0);
    for (int seg = 0; seg < NIntervals; ++seg) {
        double a = tau0 + seg * step;
        double b = a + step;
        double mid = (a + b) * 0.5;
        double half = (b - a) * 0.5;
        for (int k = 0; k < 8; ++k) {
            double tau = mid + half * glX[k];
            double tau2 = tau * tau;
            // exp(-j*tau^2) = cos(tau^2) - j*sin(tau^2)
            Complex f(std::cos(tau2), -std::sin(tau2));
            sum = sum + Complex(glW[k] * f.re, glW[k] * f.im);
        }
    }
    return Complex(sum.re * step * 0.5, sum.im * step * 0.5); // half-width scaling already in weights
}

Complex FresnelTransitionNumerical(double x)
{
    if (x < 0.0) x = 0.0;
    if (x < 1e-8) return Complex(0.0, 0.0);
    // v7.4 B19: x>10时数值积分欠采样, 切换到渐近展开 F(x)≈1+j/(2x)
    if (x > 10.0) return Complex(1.0, 0.5/x);
    double sqrtX = std::sqrt(x);
    Complex tail = FresnelIntegralTailNumerical(sqrtX);
    // F(x) = 2*j*sqrt(x)*exp(j*x) * tail
    Complex prefactor(0.0, 2.0 * sqrtX);
    Complex expJX(std::cos(x), std::sin(x));
    Complex F = prefactor * expJX;
    return Complex(F.re * tail.re - F.im * tail.im, F.re * tail.im + F.im * tail.re);
}

// ---- Helper functions ----
int NearestInt(double x) { return static_cast<int>(std::floor(x + 0.5)); }

double SafeCot(double x) {
    double s = std::sin(x);
    if (std::fabs(s) < 1e-12) return (s >= 0 ? 1e12 : -1e12);
    return std::cos(x) / s;
}

// UTD edge-diffraction coefficient D for single polarization (soft or hard)
Complex ComputeUTD_D(double k, double n, double sinBeta, double phi, double phip, double L, bool soft)
{
    double factor = -1.0 / (2.0 * n * std::sqrt(2.0 * kPi * k) * sinBeta);
    Complex pf(0.7071067811865476 * factor, -0.7071067811865476 * factor);

    double pm = phi - phip, pp = phi + phip;
    auto aFunc = [n](double beta, int sgn) {
        double tgt = (sgn > 0) ? kPi : -kPi;
        int N = NearestInt((beta + tgt) / (2.0 * kPi * n));
        double a = (2.0 * kPi * n * N - beta) * 0.5;
        double cs = std::cos(a);
        return 2.0 * cs * cs;
    };

    double denom = 2.0 * n;
    Complex T1 = Complex(SafeCot((kPi + pm) / denom), 0.0) * FresnelTransitionNumerical(k * L * aFunc(pm, 1));
    Complex T2 = Complex(SafeCot((kPi - pm) / denom), 0.0) * FresnelTransitionNumerical(k * L * aFunc(pm, -1));
    Complex T3 = Complex(SafeCot((kPi + pp) / denom), 0.0) * FresnelTransitionNumerical(k * L * aFunc(pp, 1));
    Complex T4 = Complex(SafeCot((kPi - pp) / denom), 0.0) * FresnelTransitionNumerical(k * L * aFunc(pp, -1));

    Complex sum = soft ? (T1 + T2 - T3 - T4) : (T1 + T2 + T3 + T4);
    return Complex(pf.re * sum.re - pf.im * sum.im, pf.re * sum.im + pf.im * sum.re);
}

} // namespace

bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid || node.wedge_id < 0 || !input.scene) return false;

    const auto& wedges = input.scene->wedges;
    if (node.wedge_id >= static_cast<int>(wedges.size())) return false;
    const Wedge& w = wedges[node.wedge_id];

    Vec3 eHat = Normalize(w.direction);
    if (Length(eHat) < 1e-9) return false;

    Point3 dp = node.point;
    Vec3 kOut = Normalize(node.direction);

    // Keller cone: |cos(beta0)| = |k_out . e_hat|
    double cosBeta = std::fabs(Dot(kOut, eHat));
    double beta0 = std::acos(std::min(1.0, cosBeta));
    double sinBeta = std::sin(beta0);
    if (sinBeta < 1e-9) sinBeta = 1e-9;

    // ---- C3-A: Compute incident azimuth phi' from incident direction ----
    Vec3 kInc;
    if (Length(node.incident_direction) > 0.0)
        kInc = Normalize(node.incident_direction);
    else
        kInc = MakeVec3(-kOut.x, -kOut.y, -kOut.z); // fallback

    // v7 C2修复: 边缘固定坐标系基于楔面0-face构建 (K&P 1974, Balanis Ch.12)
    // φ和φ'必须从0-face测量，否则T3/T4的cot(π±(φ+φ'))/(2n)参数错误
    Vec3 ez = eHat;
    Vec3 n0face;
    if (w.positive_face_id >= 0 && w.positive_face_id < static_cast<int>(input.scene->faces.size()))
        n0face = Normalize(input.scene->faces[w.positive_face_id].normal);
    else
        n0face = MakeVec3(1.0, 0.0, 0.0);
    // ex = 0-face内垂直于边缘的方向: Cross(n0face, ez)
    Vec3 ex = Normalize(Cross(n0face, ez));
    if (Length(ex) < 1e-9) {
        // 退化保护: n0face∥ez (不可能在合法楔面上发生, 但做安全兜底)
        double po = Dot(kOut, ez);
        ex = MakeVec3(kOut.x - po*ez.x, kOut.y - po*ez.y, kOut.z - po*ez.z);
        double le = Length(ex); if (le < 1e-9) return false;
        ex = Scale(ex, 1.0/le);
    }
    Vec3 ey = Normalize(Cross(ez, ex));
    if (Length(ey) < 1e-9) return false;

    // φ, φ' 均从0-face(ex方向)测量
    Vec3 kOutPerp = MakeVec3(kOut.x - Dot(kOut,ez)*ez.x, kOut.y - Dot(kOut,ez)*ez.y, kOut.z - Dot(kOut,ez)*ez.z);
    double plenO = Length(kOutPerp);
    if (plenO < 1e-9) return false;
    double phi = std::atan2(Dot(kOutPerp, ey), Dot(kOutPerp, ex));
    if (phi < 0.0) phi += 2.0 * kPi;

    Vec3 kIncPerp = MakeVec3(kInc.x - Dot(kInc,ez)*ez.x, kInc.y - Dot(kInc,ez)*ez.y, kInc.z - Dot(kInc,ez)*ez.z);
    double plenI = Length(kIncPerp);
    double phip = kPi;
    if (plenI > 1e-9) {
        Vec3 kIncPerpNorm = Scale(kIncPerp, 1.0 / plenI);
        phip = std::atan2(Dot(kIncPerpNorm, ey), Dot(kIncPerpNorm, ex));
        if (phip < 0.0) phip += 2.0 * kPi;
    }

    // Wavenumber
    double k = 2.0 * kPi / field.wavelength_m;

    // Wedge exterior normalization
    double alphaRad = w.wedge_angle_deg * kPi / 180.0;
    double n = (2.0 * kPi - alphaRad) / kPi;
    n = Clamp(n, 0.1, 3.0);

    // ---- C3-B: Distance parameter with correct s2 ----
    double s1 = node.segment_length_from_previous;
    if (s1 < 1e-9) s1 = 1.0;
    double s2 = 10.0;
    if (input.path && !input.path->nodes.empty()) {
        const Point3& rxPt = input.path->nodes.back().point;
        Vec3 dp2rx = Subtract(rxPt, dp);
        s2 = Length(dp2rx);
    }
    if (s2 < 1e-9) s2 = 1.0;
    // v7 C3修复: 球面波入射距离参数需包含 sin²β₀ 因子 (K&P Eq.27)
    double L = s1 * s2 * sinBeta * sinBeta / (s1 + s2);

    // UTD coefficients
    Complex Dsoft = ComputeUTD_D(k, n, sinBeta, phi, phip, L, true);
    Complex Dhard = ComputeUTD_D(k, n, sinBeta, phi, phip, L, false);

    // v7 C4修复: Soft(TE)/Hard(TM)方向与系数配对
    // 射线固定坐标系 (与phi/phi'角度的边缘固定坐标系独立)
    // D_soft→ê_φ (垂直于衍射面), D_hard→ê_β (在衍射面内, 垂直于kOut)
    Vec3 eSoft_dir = Normalize(Cross(kOut, ez));          // ⟂衍射面
    if (Length(eSoft_dir) < 1e-9) eSoft_dir = MakeVec3(1.0, 0.0, 0.0);
    Vec3 eHard_dir = Normalize(Cross(kOut, eSoft_dir));   // 在衍射面内, ⟂kOut
    Complex eS(Dot(field.polarization_vector, eSoft_dir), Dot(field.polarization_imag, eSoft_dir));
    Complex eH(Dot(field.polarization_vector, eHard_dir), Dot(field.polarization_imag, eHard_dir));
    Complex eDS = Dsoft * eS, eDH = Dhard * eH;

    // v7 C5修复: soft/hard正交分量不做标量加法
    double ampIn = std::sqrt(field.amplitude_real * field.amplitude_real +
                             field.amplitude_imag * field.amplitude_imag);
    double diffPower = eDS.NormSq() + eDH.NormSq();
    double ampMag = ampIn * std::sqrt(std::max(0.0, diffPower));

    // v5 Jones: 全复极化重构 — 先构建世界坐标复场矢量再归一化
    double prx = eDS.re * eSoft_dir.x + eDH.re * eHard_dir.x;
    double pry = eDS.re * eSoft_dir.y + eDH.re * eHard_dir.y;
    double prz = eDS.re * eSoft_dir.z + eDH.re * eHard_dir.z;
    double pix = eDS.im * eSoft_dir.x + eDH.im * eHard_dir.x;
    double piy = eDS.im * eSoft_dir.y + eDH.im * eHard_dir.y;
    double piz = eDS.im * eSoft_dir.z + eDH.im * eHard_dir.z;

    field.amplitude_real = ampMag;
    field.amplitude_imag = 0.0;
    field.power_linear = ampMag * ampMag;
    double polLen = std::sqrt(prx*prx + pry*pry + prz*prz + pix*pix + piy*piy + piz*piz);
    if (polLen > 1e-12) {
        double pInv = 1.0 / polLen;
        field.polarization_vector = MakeVec3(prx * pInv, pry * pInv, prz * pInv);
        field.polarization_imag    = MakeVec3(pix * pInv, piy * pInv, piz * pInv);
    }
    return true;
}

} // namespace rt
