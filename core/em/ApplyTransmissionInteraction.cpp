// Applies Fresnel transmission through a dielectric boundary: determines exit-side material,
// evaluates TE/TM Fresnel transmission coefficients, reconstructs transmitted field, and
// records the medium transition for subsequent attenuation tracking.

#include "ApplyTransmissionInteraction.h"
#include "../common/math/Vec3.h"
#include "../common/math/Complex.h"
#include "../common/math/MathConstants.h"
#include <cmath>

namespace rt {

namespace {

/// <summary>
/// Complex relative permittivity for a lossy dielectric.
/// eps_c = eps_r - j * sigma / (omega * eps0)
/// The imaginary part accounts for conduction losses in the medium.
/// </summary>
Complex CalcEpsC(double epsR, double sigma, double freqHz)
{
    double omega = 2.0 * kPi * freqHz;
    double imag = (omega > 0.0) ? sigma / (omega * kEpsilon0) : 0.0;
    return Complex(epsR, -imag);
}

/// <summary>
/// Fresnel TE (perpendicular / s-polarized) transmission coefficient.
/// T_TE = 2*cos(theta_i) / (cos(theta_i) + sqrt(eps_c - sin^2(theta_i)))
/// For normal incidence (cos_i = 1): T_TE = 2 / (1 + sqrt(eps_c))
/// Conservation: |T_TE|^2 + |Gamma_TE|^2 != 1 for lossy media (power absorbed).
/// </summary>
Complex FresnelTE_T(double cosI, const Complex& epsC)
{
    // sin^2(theta_i) = 1 - cos^2(theta_i)
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex cosI_c(cosI, 0.0);
    return (cosI_c + cosI_c) / (cosI_c + sqrtTerm);  // numerator = 2*cos(theta_i)
}

/// <summary>
/// Fresnel TM (parallel / p-polarized) transmission coefficient.
/// T_TM = 2*eps_c*cos(theta_i) / (eps_c*cos(theta_i) + sqrt(eps_c - sin^2(theta_i)))
/// For normal incidence: T_TM = 2*sqrt(eps_c) / (eps_c + sqrt(eps_c)) = T_TE at normal.
/// </summary>
Complex FresnelTM_T(double cosI, const Complex& epsC)
{
    Complex sin2i(1.0 - cosI * cosI, 0.0);
    Complex sqrtTerm = Sqrt(Complex(epsC.re, epsC.im) - sin2i);
    Complex e_cos(epsC.re * cosI, epsC.im * cosI);
    return (e_cos + e_cos) / (e_cos + sqrtTerm);  // numerator = 2*eps_c*cos(theta_i)
}

} // namespace

/// <summary>
/// Applies Fresnel transmission through a dielectric boundary at a path node. Determines
/// the material on the exit side of the face, evaluates TE/TM Fresnel transmission
/// coefficients, and updates the field amplitude, phase, power, and polarization.
/// Also records the new medium ID for subsequent atmospheric-attenuation tracking.
/// </summary>
/// <param name="field">Field accumulator updated with transmitted amplitude/phase/polarization and medium state.</param>
/// <param name="node">Path node carrying transmission semantics, medium IDs, and face reference.</param>
/// <param name="input">Solver input providing the material database and scene geometry.</param>
/// <returns>true if transmission was applied; false if validation, material lookup, or medium data missing.</returns>
bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input)
{
    if (!field.valid || !node.valid) return false;
    // Transmission requires complete semantic data (medium IDs, intersection side)
    if (!node.transmission_semantic_complete) return false;
    if (node.medium_in_id < 0 || node.medium_out_id < 0) return false;
    if (node.medium_in_id == node.medium_out_id) return false; // no actual boundary crossing
    if (!input.material_db || input.material_db->empty()) return false;

    // Determine which material the ray is entering (the "out" side of the face)
    // If entering from the front side, the "out" material is the back material.
    std::string outMatName;
    if (node.entered_from_front_side)
        outMatName = input.scene->faces[node.face_id].back_material_name;
    else
        outMatName = input.scene->faces[node.face_id].front_material_name;

    // Query frequency-dependent material properties of the exit-side medium
    MaterialProps props = input.material_db->QueryByName(outMatName, field.frequency_hz);
    Complex epsC = CalcEpsC(props.epsilon_r, props.sigma, field.frequency_hz);

    // Incident direction and surface normal at the hit point
    Vec3 kInc = Normalize(node.direction);   // incident wave vector
    Vec3 n = Normalize(node.surface_normal); // surface normal
    double cosI = std::fabs(Dot(kInc, n));
    if (cosI < 1e-9) cosI = 1e-9; // guard against grazing incidence

    // --- TE/TM basis decomposition (same as reflection) ---
    // eTE: perpendicular to plane of incidence (cross(k_inc, n))
    Vec3 eTE = Normalize(Cross(kInc, n));
    if (Length(eTE) < 1e-9) eTE = MakeVec3(1.0, 0.0, 0.0); // normal incidence fallback
    // eTM: in plane of incidence, perpendicular to k_inc
    Vec3 eTM = Normalize(Cross(eTE, kInc));

    // Project incident polarization onto TE/TM basis
    Complex eIncTE(Dot(field.polarization_vector, eTE), 0.0);
    Complex eIncTM(Dot(field.polarization_vector, eTM), 0.0);

    // Evaluate Fresnel transmission coefficients (complex, include phase shift)
    Complex tTE = FresnelTE_T(cosI, epsC);
    Complex tTM = FresnelTM_T(cosI, epsC);

    // Apply transmission coefficients to TE/TM components
    Complex eTransTE = tTE * eIncTE;
    Complex eTransTM = tTM * eIncTM;

    // Reconstruct transmitted polarization vector
    // Note: unlike reflection, the transmitted ray direction does not change
    // geometrically; only the amplitude and phase are modified.
    double txX = eTransTE.re * eTE.x + eTransTM.re * eTM.x;
    double txY = eTransTE.re * eTE.y + eTransTM.re * eTM.y;
    double txZ = eTransTE.re * eTE.z + eTransTM.re * eTM.z;

    // Update field amplitude using RMS average of TE and TM transmission magnitudes
    Complex amp(field.amplitude_real, field.amplitude_imag);
    // |T_avg| = sqrt((|T_TE|^2 + |T_TM|^2) / 2) -- RMS of magnitudes
    double tAvg = std::sqrt((tTE.NormSq() + tTM.NormSq()) * 0.5);
    amp = amp * Complex(tAvg, 0.0);  // scale amplitude by RMS transmission magnitude
    // Average phase shift from TE and TM transmission coefficients
    double phaseShift = (tTE.Arg() + tTM.Arg()) * 0.5;

    // Write updated field state
    field.amplitude_real = amp.re;
    field.amplitude_imag = amp.im;
    field.phase_rad += phaseShift;       // accumulate transmission phase shift
    field.power_linear = amp.NormSq();   // power = |amplitude|^2
    field.polarization_vector = MakeVec3(txX, txY, txZ);

    // --- Medium transition ---
    // Update current medium for subsequent attenuation accumulation in ApplyFreeSpaceSegment.
    // The attenuation constant (Np/m) for the new medium should be set by the pipeline
    // based on the medium's conductivity/loss properties.
    field.current_medium_id = node.medium_out_id;
    field.last_transmission_medium_in_id = node.medium_in_id;
    field.last_transmission_medium_out_id = node.medium_out_id;
    field.transmission_semantic_consumed = true; // mark that a transmission has been applied

    return true;
}

} // namespace rt
