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

    // Edge-fixed frame: ez = edge, ex = perp(kOut, ez), ey = ez x ex
    Vec3 ez = eHat;
    double projO = Dot(kOut, ez);
    Vec3 kOutPerp = MakeVec3(kOut.x - projO * ez.x, kOut.y - projO * ez.y, kOut.z - projO * ez.z);
    double plenO = Length(kOutPerp);
    if (plenO < 1e-9) return false;
    Vec3 ex = Scale(kOutPerp, 1.0 / plenO);
    Vec3 ey = Normalize(Cross(ez, ex));

    // Diffraction angle phi (outgoing)
    double phi = std::atan2(Dot(kOut, ey), Dot(kOut, ex));
    if (phi < 0.0) phi += 2.0 * kPi;

    // Incident angle phi' from incident direction
    double projI = Dot(kInc, ez);
    Vec3 kIncPerp = MakeVec3(kInc.x - projI * ez.x, kInc.y - projI * ez.y, kInc.z - projI * ez.z);
    double plenI = Length(kIncPerp);
    double phip = kPi; // default
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
    double L = s1 * s2 / (s1 + s2);

    // UTD coefficients
    Complex Dsoft = ComputeUTD_D(k, n, sinBeta, phi, phip, L, true);
    Complex Dhard = ComputeUTD_D(k, n, sinBeta, phi, phip, L, false);

    // Soft/hard decomposition (v5 Jones: 复投影)
    Vec3 eSoft = ex;
    Vec3 eHard = Normalize(Cross(eSoft, kOut));
    Complex eS(Dot(field.polarization_vector, eSoft), Dot(field.polarization_imag, eSoft));
    Complex eH(Dot(field.polarization_vector, eHard), Dot(field.polarization_imag, eHard));
    Complex eDS = Dsoft * eS, eDH = Dhard * eH;

    Complex amp(field.amplitude_real, field.amplitude_imag);
    Complex ampDiff = eDS + eDH;
    amp = amp * ampDiff;

    field.amplitude_real = amp.re;
    field.amplitude_imag = amp.im;
    field.power_linear = amp.NormSq();
    // v5 Jones: 全复极化重构
    field.polarization_vector = MakeVec3(
        eDS.re * eSoft.x + eDH.re * eHard.x,
        eDS.re * eSoft.y + eDH.re * eHard.y,
        eDS.re * eSoft.z + eDH.re * eHard.z);
    field.polarization_imag = MakeVec3(
        eDS.im * eSoft.x + eDH.im * eHard.x,
        eDS.im * eSoft.y + eDH.im * eHard.y,
        eDS.im * eSoft.z + eDH.im * eHard.z);
    return true;
}

} // namespace rt
