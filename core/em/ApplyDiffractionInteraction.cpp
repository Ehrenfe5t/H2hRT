// Applies UTD (Uniform Theory of Diffraction) edge diffraction at a wedge node:
// builds the edge-fixed frame, computes the Keller cone, evaluates soft/hard UTD
// diffraction coefficients (Kouyoumjian-Pathak), and reconstructs the diffracted field.

#include "ApplyDiffractionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// UTD Fresnel transition function (Kouyoumjian transition function).
/// Piecewise polynomial approximation of the Fresnel integral:
///   F(x) = 2*j*sqrt(x)*exp(j*x) * integral(sqrt(x), inf) e^(-j*tau^2) dtau
/// appearing in the UTD diffraction coefficients for wedge diffraction.
/// Three regimes: small x (Taylor series), medium x, large x (asymptotic).
/// The real part models the amplitude transition across shadow boundaries;
/// the imaginary part models the phase.
/// </summary>
Complex FresnelTransition(double x)
{
    if (x < 0.0) x = 0.0;
    if (x < 1e-8) return Complex(0.0, 0.0); // vanishing for very small arguments
    double sqrtX = std::sqrt(x);
    if (x <= 0.8) {
        // Small-x regime: series expansion of sqrt(pi*x) * Fresnel integrals
        double x2 = x*x, x3 = x2*x, x4 = x3*x;
        return Complex(std::sqrt(kPi*x)*(1.0 - x/3.0 + x2/10.0 - x3/42.0 + x4/216.0),
                       2.0*x*(1.0 - 2.0*x/5.0 + 4.0*x2/27.0 - 8.0*x3/165.0));
    } else if (x <= 10.0) {
        // Medium-x regime: rational approximation
        double xi = 1.0/x, poly = 0.125*xi;
        return Complex(1.0 - poly, poly*0.75);
    } else {
        // Large-x regime: asymptotic leading-term approximation
        double xi = 1.0/x;
        return Complex(1.0 - 0.75*xi*xi, 0.5*xi);
    }
}

/// <summary>Rounds a double to the nearest integer via floor(x+0.5). Used for UTD integer-N computation.</summary>
int NearestInt(double x) { return static_cast<int>(std::floor(x + 0.5)); }

/// <summary>
/// Safe cotangent: returns cos(x)/sin(x), clamped to +/-1e12 when sin(x) approaches zero,
/// preventing division-by-zero singularities in the UTD cotangent terms.
/// </summary>
double SafeCot(double x) {
    double s = std::sin(x);
    if (std::fabs(s) < 1e-12) return (s >= 0 ? 1e12 : -1e12);
    return std::cos(x)/s;
}

/// <summary>
/// UTD edge-diffraction coefficient D for a single polarization (soft or hard boundary).
///
/// This implements the Kouyoumjian-Pathak heuristic UTD diffraction coefficient for a
/// perfectly conducting wedge:
///   D(k, n, beta0, phi, phi', L, s/h) =
///     -exp(-j*pi/4) / (2*n*sqrt(2*pi*k)*sin(beta0)) *
///       [cot(...)*F(k*L*a(...)) +/- cot(...)*F(k*L*a(...)) ...]
///
/// Four terms:
///   T1 = cot((pi + phi-phi')/(2n)) * F(k*L*a+(phi-phi'))
///   T2 = cot((pi - phi+phi')/(2n)) * F(k*L*a-(phi-phi'))
///   T3 = cot((pi + phi+phi')/(2n)) * F(k*L*a+(phi+phi'))
///   T4 = cot((pi - phi-phi')/(2n)) * F(k*L*a-(phi+phi'))
///
/// Soft polarization (Dirichlet B.C.): sum = T1 + T2 - T3 - T4 (minus on T3,T4)
/// Hard polarization (Neumann B.C.):  sum = T1 + T2 + T3 + T4 (all positive)
///
/// a+(beta) and a-(beta) are the large-argument functions that determine
/// whether Fresnel transition is in the lit or shadow region.
/// </summary>
/// <param name="k">Wavenumber = 2*pi/lambda.</param>
/// <param name="n">Wedge exterior angle factor: n = (2*pi - alpha_wedge) / pi.</param>
/// <param name="sinBeta">sin(beta0), where beta0 is the Keller-cone half-angle.</param>
/// <param name="phi">Diffracted-ray azimuthal angle in the edge-perpendicular plane.</param>
/// <param name="phip">Incident-ray azimuthal angle (phip = pi for backward diffraction).</param>
/// <param name="L">UTD distance parameter = s1*s2/(s1+s2) * sin^2(beta0).</param>
/// <param name="soft">true for soft/Dirichlet (E parallel to edge); false for hard/Neumann.</param>
/// <returns>Complex UTD diffraction coefficient D_soft or D_hard.</returns>
Complex ComputeUTD_D(double k, double n, double sinBeta, double phi, double phip, double L, bool soft)
{
    // Pre-factor: -exp(-j*pi/4) / (2*n*sqrt(2*pi*k)*sin(beta0))
    // exp(-j*pi/4) = (1 - j)/sqrt(2) = 0.7071*(1 - j)
    double factor = -1.0/(2.0*n*std::sqrt(2.0*kPi*k)*sinBeta);
    Complex pf(0.7071067811865476*factor, -0.7071067811865476*factor);

    // Angular differences used in the four UTD terms
    double pm = phi - phip;  // phi - phi'
    double pp = phi + phip;  // phi + phi'

    // a+(beta) and a-(beta): large-argument functions = 2*cos^2((2*pi*n*N - beta)/2)
    // Determines which branch of the cotangent is active and whether the
    // Fresnel transition function argument is large enough for asymptotic behavior.
    // sgn > 0 gives a+ (uses +pi), sgn < 0 gives a- (uses -pi).
    auto aFunc = [n](double beta, int sgn) {
        double tgt = (sgn > 0) ? kPi : -kPi;
        // N = nearest integer to (beta +/- pi) / (2*pi*n)
        int N = NearestInt((beta + tgt)/(2.0*kPi*n));
        double a = (2.0*kPi*n*N - beta)*0.5;
        double cs = std::cos(a);
        return 2.0*cs*cs;  // 2*cos^2(a) = 1 + cos(2a)
    };

    double denom = 2.0*n;
    // Four terms of the UTD coefficient (see Kouyoumjian-Pathak formulation)
    Complex T1 = Complex(SafeCot((kPi + pm)/denom), 0.0) * FresnelTransition(k*L*aFunc(pm, 1));
    Complex T2 = Complex(SafeCot((kPi - pm)/denom), 0.0) * FresnelTransition(k*L*aFunc(pm, -1));
    Complex T3 = Complex(SafeCot((kPi + pp)/denom), 0.0) * FresnelTransition(k*L*aFunc(pp, 1));
    Complex T4 = Complex(SafeCot((kPi - pp)/denom), 0.0) * FresnelTransition(k*L*aFunc(pp, -1));

    // Soft: minus on T3,T4 (Dirichlet B.C., E-field parallel to edge)
    // Hard: plus  on T3,T4 (Neumann B.C., H-field parallel to edge)
    Complex sum = soft ? (T1 + T2 - T3 - T4) : (T1 + T2 + T3 + T4);
    // Multiply by the pre-factor (complex multiplication)
    return Complex(pf.re*sum.re - pf.im*sum.im, pf.re*sum.im + pf.im*sum.re);
}

} // namespace

/// <summary>
/// Applies UTD edge diffraction at a wedge-diffraction path node. Retrieves the wedge geometry,
/// builds the edge-fixed coordinate frame, computes the Keller-cone diffraction angle, evaluates
/// soft and hard UTD diffraction coefficients, decomposes the incident polarization into
/// soft/hard components, and reconstructs the diffracted complex amplitude, phase, power,
/// and polarization vector.
/// </summary>
/// <param name="field">Field accumulator updated with diffracted amplitude/phase/polarization.</param>
/// <param name="node">Path node containing the wedge ID, diffraction point, and outgoing direction.</param>
/// <param name="input">Solver input providing the scene with wedge geometry.</param>
/// <returns>true if diffraction was applied; false if validation or geometry data missing.</returns>
bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid || node.wedge_id < 0 || !input.scene) return false;

    const auto& wedges = input.scene->wedges;
    if (node.wedge_id >= static_cast<int>(wedges.size())) return false;
    const Wedge& w = wedges[node.wedge_id];

    // Edge unit direction vector in world coordinates
    Vec3 eHat = Normalize(w.direction);
    if (Length(eHat) < 1e-9) return false;

    // Diffraction point and outgoing (diffracted) ray direction
    Point3 dp = node.point;
    Vec3 kOut = Normalize(node.direction);

    // --- Keller cone ---
    // In UTD, the diffracted ray must lie on the Keller cone: the incident and
    // diffracted rays make the same angle beta0 with the edge direction.
    // cos(beta0) = |k_out . e_hat|, sin(beta0) = sqrt(1 - cos^2)
    double cosBeta = std::fabs(Dot(kOut, eHat));
    double beta0 = std::acos(std::min(1.0, cosBeta)); // Keller cone half-angle
    double sinBeta = std::sin(beta0);
    if (sinBeta < 1e-9) sinBeta = 1e-9; // guard: grazing along edge

    // --- Edge-fixed coordinate frame ---
    // ez = edge direction
    // ex = perpendicular component of kOut in the edge-normal plane (pointing away from edge)
    // ey = cross(ez, ex) completes right-handed frame
    Vec3 ez = eHat;
    // Subtract the component of kOut parallel to the edge to get the perpendicular part
    Vec3 kOutPerp = MakeVec3(kOut.x - Dot(kOut, ez)*ez.x,
                              kOut.y - Dot(kOut, ez)*ez.y,
                              kOut.z - Dot(kOut, ez)*ez.z);
    double plen = Length(kOutPerp);
    if (plen < 1e-9) return false; // ray is parallel to edge (no diffraction)
    Vec3 ex = Scale(kOutPerp, 1.0/plen); // normalized perpendicular direction
    Vec3 ey = Normalize(Cross(ez, ex));   // completes right-handed frame

    // Diffraction azimuthal angle phi in the edge-perpendicular (ex, ey) plane
    // phi = atan2(kOut . ey, kOut . ex), wrapped to [0, 2*pi)
    double phi = std::atan2(Dot(kOut, ey), Dot(kOut, ex));
    if (phi < 0.0) phi += 2.0*kPi;

    // Incident azimuthal angle phi': for backward diffraction from the wedge,
    // the incident ray comes approximately from the opposite side (phi' = pi).
    double phip = kPi; // approximate incident azimuth

    // Wavenumber: k = 2*pi / lambda
    double k = 2.0*kPi/field.wavelength_m;

    // --- Wedge exterior angle factor n ---
    // n = (2*pi - alpha) / pi, where alpha is the interior wedge angle in radians.
    // n = 2 for a half-plane (alpha = 0), n = 1 for a right-angle wedge (alpha = pi/2).
    // Clamped to [0.1, 3.0] for numerical stability.
    double alphaRad = w.wedge_angle_deg*kPi/180.0;
    double n = (2.0*kPi - alphaRad)/kPi;
    n = Clamp(n, 0.1, 3.0);

    // --- UTD distance parameter L ---
    // L = s1 * s2 / (s1 + s2) where s1 is the distance from the previous interaction
    // to the diffraction point and s2 is the residual distance to the receiver.
    // For spherical wave incidence: L = s1*s2/(s1+s2) * sin^2(beta0)
    double s1 = node.segment_length_from_previous;
    if (s1 < 1e-9) s1 = 1.0;
    double s2 = 10.0; // residual path length estimate (placeholder)
    double L = s1*s2/(s1+s2);
    if (L < 1e-9) L = 1.0;

    // --- Compute UTD diffraction coefficients ---
    // Soft polarization (Dsoft): E-field component parallel to the edge (Dirichlet B.C.)
    // Hard polarization (Dhard): H-field component parallel to the edge (Neumann B.C.)
    Complex Dsoft = ComputeUTD_D(k, n, sinBeta, phi, phip, L, true);
    Complex Dhard = ComputeUTD_D(k, n, sinBeta, phi, phip, L, false);

    // --- Soft/hard decomposition of incident polarization ---
    // eSoft: direction parallel to the edge (ez direction = eHat)
    // eHard: direction perpendicular to both the edge and the outgoing ray
    // Soft polarization = E-field parallel to edge (Dirichlet)
    // Hard polarization = H-field parallel to edge (Neumann), equivalent to
    //   E-field in the plane spanned by edge and diffraction direction.
    Vec3 eSoft = ex;                         // soft polarization basis vector
    Vec3 eHard = Normalize(Cross(eSoft, kOut)); // hard polarization basis vector
    // Project incident polarization onto soft and hard basis
    Complex eS(Dot(field.polarization_vector, eSoft), 0.0);
    Complex eH(Dot(field.polarization_vector, eHard), 0.0);
    // Apply diffraction coefficients per polarization component
    Complex eDS = Dsoft*eS;  // diffracted soft component
    Complex eDH = Dhard*eH;  // diffracted hard component

    // Update field amplitude using RMS average of soft and hard coefficient magnitudes
    Complex amp(field.amplitude_real, field.amplitude_imag);
    // |D_avg| = sqrt((|Dsoft|^2 + |Dhard|^2) / 2) -- RMS of magnitudes
    double dAvg = std::sqrt((Dsoft.NormSq() + Dhard.NormSq())*0.5);
    if (dAvg > 1e-12) amp = amp*Complex(dAvg, 0.0); // scale by RMS diffraction magnitude
    // Average phase shift from soft and hard diffraction coefficients
    double phShift = (Dsoft.Arg() + Dhard.Arg())*0.5;

    // Write updated field state
    field.amplitude_real = amp.re; field.amplitude_imag = amp.im;
    field.phase_rad += phShift;       // accumulate diffraction phase shift
    field.power_linear = amp.NormSq(); // power = |amplitude|^2
    // Reconstruct diffracted polarization vector from soft/hard components
    field.polarization_vector = MakeVec3(
        eDS.re*eSoft.x + eDH.re*eHard.x,
        eDS.re*eSoft.y + eDH.re*eHard.y,
        eDS.re*eSoft.z + eDH.re*eHard.z);
    return true;
}

} // namespace rt
