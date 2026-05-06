// Applies Fresnel reflection at a path node: decomposes incident polarization into
// TE/TM components, evaluates complex Fresnel coefficients for the surface material,
// and reconstructs reflected amplitude, phase, power, and polarization.

#include "ApplyReflectionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include "../common/material/MaterialDatabase.h"
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Complex relative permittivity for a lossy dielectric.
/// Formula: eps_c = eps_r - j * sigma / (omega * eps0)
/// where omega = 2*pi*f is the angular frequency, sigma is conductivity (S/m),
/// and eps0 is the vacuum permittivity. The imaginary part represents dielectric loss.
/// </summary>
Complex CalcEpsC(double epsR, double sigma, double freqHz)
{
    double omega = 2.0 * kPi * freqHz;
    // Imaginary part: conduction loss term = sigma / (omega * eps0)
    double imag = (omega > 0.0) ? sigma / (omega * kEpsilon0) : 0.0;
    return Complex(epsR, -imag);  // -j convention for time-harmonic e^{j*omega*t}
}

/// <summary>
/// Fresnel TE (perpendicular / s-polarized) reflection coefficient.
/// Gamma_TE = (cos(theta_i) - sqrt(eps_c - sin^2(theta_i))) / (cos(theta_i) + sqrt(eps_c - sin^2(theta_i)))
/// TE = electric field perpendicular to the plane of incidence (out of plane).
/// For a perfect conductor (eps_c -> -inf*j), Gamma_TE -> -1 (phase reversal).
/// </summary>
Complex FresnelTE(double cosI, const Complex& epsC)
{
    // sin^2(theta_i) = 1 - cos^2(theta_i)
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    // sqrt(eps_c - sin^2(theta_i)) -- the complex refraction term
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex cosI_c(cosI, 0.0);
    return (cosI_c - sqrtTerm) / (cosI_c + sqrtTerm);
}

/// <summary>
/// Fresnel TM (parallel / p-polarized) reflection coefficient.
/// Gamma_TM = (eps_c*cos(theta_i) - sqrt(eps_c - sin^2(theta_i))) / (eps_c*cos(theta_i) + sqrt(eps_c - sin^2(theta_i)))
/// TM = electric field parallel to the plane of incidence (in plane).
/// Brewster angle occurs when the numerator -> 0 (for lossless dielectrics).
/// </summary>
Complex FresnelTM(double cosI, const Complex& epsC)
{
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex epsC_cosI(epsC.re * cosI, epsC.im * cosI);
    return (epsC_cosI - sqrtTerm) / (epsC_cosI + sqrtTerm);
}

} // namespace

/// <summary>
/// Applies Fresnel reflection at a path node. Decomposes the incident polarization into TE/TM
/// components relative to the surface normal and incident direction, evaluates the Fresnel
/// reflection coefficients for the surface material, and reconstructs the reflected complex
/// amplitude, phase shift, power, and polarization vector.
/// </summary>
/// <param name="field">Field accumulator updated with reflected amplitude/phase/polarization.</param>
/// <param name="node">Path node containing the hit point, incident direction, surface normal, and material references.</param>
/// <param name="input">Solver input providing the material database.</param>
/// <returns>true if reflection was applied; false if field/node invalid or material lookup failed.</returns>
bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node,
                                const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    if (!input.material_db || input.material_db->empty()) return false;

    // Extract incident wave vector (unit direction) and surface normal
    Vec3 kInc = Normalize(node.direction);   // incident ray direction
    Vec3 n = Normalize(node.surface_normal); // surface normal (pointing outward)
    // Cosine of incidence angle: cos(theta_i) = |k_inc . n|
    double cosI = std::fabs(Dot(kInc, n));
    if (cosI < 1e-9) cosI = 1e-9; // guard against grazing incidence division by zero

    // Determine reflecting material: use front_material if ray hits front side
    // For now use the face's back material (exterior reflection)
    std::string matName;
    if (node.front_material_id >= 0) {
        // Material name should come from the face via the scene. Use a default placeholder.
        matName = "Concrete";
    }

    // Query frequency-dependent material properties (epsilon_r, sigma) from database
    MaterialProps props = input.material_db->QueryByName(matName, field.frequency_hz);

    // Compute complex permittivity including conduction losses
    Complex epsC = CalcEpsC(props.epsilon_r, props.sigma, field.frequency_hz);

    // --- TE/TM basis decomposition ---
    // eTE: unit vector perpendicular to the plane of incidence (cross(k_inc, n))
    // TE = "transverse electric" = E-field perpendicular to plane of incidence
    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) {
        // Normal incidence: plane of incidence is undefined, TE/TM degenerate.
        // Pick an arbitrary direction in the tangent plane.
        eTE = MakeVec3(1.0, 0.0, 0.0);
    }
    // eTM: unit vector in the plane of incidence, perpendicular to k_inc
    // TM = "transverse magnetic" = E-field parallel to plane of incidence
    Vec3 eTM = Normalize(Cross(eTE, kInc));

    // Project incident polarization onto TE and TM basis vectors
    Complex eIncTE(Dot(field.polarization_vector, eTE), 0.0);
    Complex eIncTM(Dot(field.polarization_vector, eTM), 0.0);

    // Evaluate Fresnel reflection coefficients (complex, including phase shift)
    Complex gammaTE = FresnelTE(cosI, epsC);
    Complex gammaTM = FresnelTM(cosI, epsC);

    // Apply Fresnel coefficients to TE/TM components of incident field
    Complex eRefTE = gammaTE * eIncTE;
    Complex eRefTM = gammaTM * eIncTM;

    // Reconstruct reflected polarization vector from TE/TM components
    double refX = eRefTE.re * eTE.x + eRefTM.re * eTM.x;
    double refY = eRefTE.re * eTE.y + eRefTM.re * eTM.y;
    double refZ = eRefTE.re * eTE.z + eRefTM.re * eTM.z;

    // Update field amplitude using RMS average of TE and TM reflection magnitudes
    Complex amp(field.amplitude_real, field.amplitude_imag);
    // |Gamma_avg| = sqrt((|Gamma_TE|^2 + |Gamma_TM|^2) / 2) -- RMS of magnitudes
    double rTE = gammaTE.Norm();
    double rTM = gammaTM.Norm();
    double rAvg = std::sqrt((rTE*rTE + rTM*rTM) * 0.5);
    amp = amp * Complex(rAvg, 0.0);  // scale amplitude by RMS reflection magnitude
    // Average phase shift from TE and TM reflection coefficients
    double phaseShift = (gammaTE.Arg() + gammaTM.Arg()) * 0.5;

    // Write updated field state back
    field.amplitude_real = amp.re;
    field.amplitude_imag = amp.im;
    field.phase_rad += phaseShift;       // accumulate reflection phase shift
    field.power_linear = amp.NormSq();   // power = |amplitude|^2
    field.polarization_vector = MakeVec3(refX, refY, refZ);

    return true;
}

} // namespace rt
