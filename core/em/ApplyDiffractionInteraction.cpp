// Finite-conductivity UTD edge diffraction with coherent Jones propagation.

#include "ApplyDiffractionInteraction.h"
#include "FresnelInterface.h"
#include "../common/math/Complex.h"
#include "../common/math/ComplexVec3.h"
#include "../common/math/MathConstants.h"
#include "../common/math/Vec3.h"

#include <cmath>
#include <string>

namespace rt {
namespace {

Complex FresnelIntegralNumerical(double argument)
{
    static const double glX[8] = {
        -0.960289856497536, -0.796666477413627, -0.525532409916329, -0.183434642495650,
         0.183434642495650,  0.525532409916329,  0.796666477413627,  0.960289856497536};
    static const double glW[8] = {
         0.101228536290376,  0.222381034453374,  0.313706645877887,  0.362683783378362,
         0.362683783378362,  0.313706645877887,  0.222381034453374,  0.101228536290376};

    const double upper = std::sqrt(2.0 * argument / kPi);
    const int intervalCount = std::max(1, static_cast<int>(std::ceil(upper / 0.25)));
    const double step = upper / intervalCount;
    Complex sum;
    for (int segment = 0; segment < intervalCount; ++segment) {
        const double a = segment * step;
        const double midpoint = a + 0.5 * step;
        for (int index = 0; index < 8; ++index) {
            const double value = midpoint + 0.5 * step * glX[index];
            const double phase = 0.5 * kPi * value * value;
            sum += Complex(glW[index] * std::cos(phase),
                           glW[index] * std::sin(phase));
        }
    }
    return sum * (0.5 * step);
}

int NearestInt(double value)
{
    return static_cast<int>(std::floor(value + 0.5));
}

double SafeCot(double value)
{
    const double sine = std::sin(value);
    if (std::fabs(sine) < 1.0e-12) return sine >= 0.0 ? 1.0e12 : -1.0e12;
    return std::cos(value) / sine;
}

struct UtdTerms {
    Complex d1;
    Complex d2;
    Complex d3;
    Complex d4;
};

UtdTerms ComputeUtdTerms(double waveNumber, double wedgeIndex, double sinBeta,
                         double phi, double phiPrime, double distanceParameter)
{
    const double scalar = -1.0 /
        (2.0 * wedgeIndex * std::sqrt(2.0 * kPi * waveNumber) * sinBeta);
    const Complex factor = Complex(0.7071067811865476, -0.7071067811865476) * scalar;
    const double difference = phi - phiPrime;
    const double sum = phi + phiPrime;

    auto transitionArgument = [wedgeIndex](double beta, bool plus) {
        const double offset = plus ? kPi : -kPi;
        const int nearest = NearestInt((beta + offset) / (2.0 * kPi * wedgeIndex));
        const double angle = (2.0 * kPi * wedgeIndex * nearest - beta) * 0.5;
        const double cosine = std::cos(angle);
        return 2.0 * cosine * cosine;
    };

    const double denominator = 2.0 * wedgeIndex;
    UtdTerms terms;
    terms.d1 = factor * SafeCot((kPi + difference) / denominator) *
        EvaluateUtdTransition(waveNumber * distanceParameter * transitionArgument(difference, true));
    terms.d2 = factor * SafeCot((kPi - difference) / denominator) *
        EvaluateUtdTransition(waveNumber * distanceParameter * transitionArgument(difference, false));
    terms.d3 = factor * SafeCot((kPi + sum) / denominator) *
        EvaluateUtdTransition(waveNumber * distanceParameter * transitionArgument(sum, true));
    terms.d4 = factor * SafeCot((kPi - sum) / denominator) *
        EvaluateUtdTransition(waveNumber * distanceParameter * transitionArgument(sum, false));
    return terms;
}

struct ComplexMatrix2 {
    Complex m00;
    Complex m01;
    Complex m10;
    Complex m11;
};

ComplexMatrix2 Add(const ComplexMatrix2& left, const ComplexMatrix2& right)
{
    return {left.m00 + right.m00, left.m01 + right.m01,
            left.m10 + right.m10, left.m11 + right.m11};
}

ComplexMatrix2 Multiply(const ComplexMatrix2& left, const ComplexMatrix2& right)
{
    return {
        left.m00 * right.m00 + left.m01 * right.m10,
        left.m00 * right.m01 + left.m01 * right.m11,
        left.m10 * right.m00 + left.m11 * right.m10,
        left.m10 * right.m01 + left.m11 * right.m11
    };
}

ComplexMatrix2 Scale(const ComplexMatrix2& matrix, double scalar)
{
    return {matrix.m00 * scalar, matrix.m01 * scalar,
            matrix.m10 * scalar, matrix.m11 * scalar};
}

ComplexMatrix2 JonesRotator(const Vec3& propagationDirection,
                            const Vec3& currentFirstBasis,
                            const Vec3& targetFirstBasis)
{
    const double cosine = Dot(currentFirstBasis, targetFirstBasis);
    const double sine = Dot(propagationDirection, Cross(currentFirstBasis, targetFirstBasis));
    return {Complex(cosine, 0.0), Complex(sine, 0.0),
            Complex(-sine, 0.0), Complex(cosine, 0.0)};
}

struct FaceReflection {
    bool materialResolved = false;
    std::string materialName;
    Complex te = Complex(-1.0, 0.0);
    Complex tm = Complex(1.0, 0.0);
};

FaceReflection ResolveFaceReflection(const std::string& materialName,
                                     double cosineIncident,
                                     const FieldAccumulator& field,
                                     const EMSolverInput& input)
{
    FaceReflection result;
    result.materialName = materialName;
    if (!input.material_db || input.material_db->empty() || materialName.empty() ||
        !input.material_db->HasMaterial(materialName)) {
        return result;
    }

    MaterialProps vacuum;
    vacuum.name = "Vacuum";
    const MaterialProps material = input.material_db->QueryByName(materialName, field.frequency_hz);
    const FresnelInterfaceCoefficients coefficients = EvaluateFresnelInterface(
        vacuum, material, std::max(1.0e-9, std::min(1.0, cosineIncident)), field.frequency_hz);
    result.materialResolved = true;
    result.te = coefficients.reflection_te;
    result.tm = coefficients.reflection_tm;
    return result;
}

Vec3 SafeTransverseBasis(const Vec3& propagationDirection, const Vec3& preferred)
{
    Vec3 result = Normalize(preferred);
    if (Length(result) > 0.5 && std::fabs(Dot(result, propagationDirection)) < 1.0e-7) return result;
    const Vec3 seed = std::fabs(propagationDirection.x) < 0.9
        ? MakeVec3(1.0, 0.0, 0.0) : MakeVec3(0.0, 1.0, 0.0);
    return Normalize(Cross(propagationDirection, seed));
}

ComplexMatrix2 BuildFaceTerm(const Vec3& incidentDirection,
                             const Vec3& outgoingDirection,
                             const Vec3& incidentEdgeBasis,
                             const Vec3& outgoingEdgeBasis,
                             const Vec3& faceNormal,
                             const FaceReflection& reflection,
                             const Complex& utdTerm)
{
    const Vec3 faceTe = SafeTransverseBasis(incidentDirection,
                                             Cross(incidentDirection, faceNormal));
    const ComplexMatrix2 toFace = JonesRotator(incidentDirection, incidentEdgeBasis, faceTe);
    const ComplexMatrix2 fromFace = JonesRotator(outgoingDirection, faceTe, outgoingEdgeBasis);
    const ComplexMatrix2 diagonal = {
        reflection.te * utdTerm, Complex(), Complex(), reflection.tm * utdTerm};
    return Multiply(fromFace, Multiply(diagonal, toFace));
}

ComplexVec3 BuildLegacyComplexField(const FieldAccumulator& field)
{
    const Complex amplitude(field.amplitude_real, field.amplitude_imag);
    return ComplexVec3(
        amplitude * Complex(field.polarization_vector.x, field.polarization_imag.x),
        amplitude * Complex(field.polarization_vector.y, field.polarization_imag.y),
        amplitude * Complex(field.polarization_vector.z, field.polarization_imag.z));
}

} // namespace

Complex EvaluateUtdTransition(double x)
{
    x = std::max(0.0, x);
    if (x < 1.0e-12) return Complex();
    if (x > 50.0) return Complex(1.0, 0.5 / x);

    // F(x)=sqrt(pi*x/2)*exp(j*x)*[(1+j)-2j*conj(C+jS)].
    // Integrating C and S on their finite interval avoids truncating the
    // oscillatory tail integral, whose error grows for moderate/large x.
    const Complex fresnel = FresnelIntegralNumerical(x);
    const Complex bracket(1.0 - 2.0 * fresnel.im,
                          1.0 - 2.0 * fresnel.re);
    return CExp(x) * bracket * std::sqrt(0.5 * kPi * x);
}

bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node,
                                 const EMSolverInput& input)
{
    field.last_diffraction = DiffractionFieldDiagnostics{};
    if (!field.valid || !node.valid || node.wedge_id < 0 || !input.scene ||
        field.wavelength_m <= 0.0 || field.frequency_hz <= 0.0) return false;
    if (node.wedge_id >= static_cast<int>(input.scene->wedges.size())) return false;

    const Wedge& wedge = input.scene->wedges[node.wedge_id];
    const Vec3 edgeDirection = Normalize(wedge.direction);
    const Vec3 outgoingDirection = Normalize(node.direction);
    const Vec3 incidentDirection = Length(node.incident_direction) > 0.5
        ? Normalize(node.incident_direction) : Scale(outgoingDirection, -1.0);
    if (Length(edgeDirection) < 0.5 || Length(outgoingDirection) < 0.5 ||
        Length(incidentDirection) < 0.5) return false;

    const int zeroFaceId = wedge.zero_face_id >= 0 ? wedge.zero_face_id : wedge.positive_face_id;
    const int nFaceId = zeroFaceId == wedge.positive_face_id
        ? wedge.negative_face_id : wedge.positive_face_id;
    if (zeroFaceId < 0 || nFaceId < 0 ||
        zeroFaceId >= static_cast<int>(input.scene->faces.size()) ||
        nFaceId >= static_cast<int>(input.scene->faces.size())) return false;

    const Face& zeroFace = input.scene->faces[zeroFaceId];
    const Face& nFace = input.scene->faces[nFaceId];
    const Vec3 zeroFaceNormal = Normalize(zeroFace.normal);
    const Vec3 nFaceNormal = Normalize(nFace.normal);
    if (Length(zeroFaceNormal) < 0.5 || Length(nFaceNormal) < 0.5) return false;

    const std::string& zeroMaterial = zeroFaceId == wedge.positive_face_id
        ? wedge.positive_material_name : wedge.negative_material_name;
    const std::string& nMaterial = nFaceId == wedge.positive_face_id
        ? wedge.positive_material_name : wedge.negative_material_name;

    const double cosBeta = std::fabs(Dot(outgoingDirection, edgeDirection));
    const double beta = std::acos(std::min(1.0, cosBeta));
    const double sinBeta = std::max(1.0e-9, std::sin(beta));

    const Vec3 zeroFaceTangent = Normalize(Cross(zeroFaceNormal, edgeDirection));
    const Vec3 zeroFacePerpendicular = Normalize(Cross(edgeDirection, zeroFaceTangent));
    if (Length(zeroFaceTangent) < 0.5 || Length(zeroFacePerpendicular) < 0.5) return false;

    const Vec3 outgoingProjection = Normalize(Subtract(
        outgoingDirection, Scale(edgeDirection, Dot(outgoingDirection, edgeDirection))));
    const Vec3 incidentProjection = Normalize(Subtract(
        incidentDirection, Scale(edgeDirection, Dot(incidentDirection, edgeDirection))));
    if (Length(outgoingProjection) < 0.5 || Length(incidentProjection) < 0.5) return false;

    double phi = std::atan2(Dot(outgoingProjection, zeroFacePerpendicular),
                            Dot(outgoingProjection, zeroFaceTangent));
    if (phi < 0.0) phi += kTwoPi;

    // UTD measures phi' using the direction from the diffraction point to the source.
    const Vec3 towardSource = Scale(incidentProjection, -1.0);
    double phiPrime = std::atan2(Dot(towardSource, zeroFacePerpendicular),
                                 Dot(towardSource, zeroFaceTangent));
    if (phiPrime < 0.0) phiPrime += kTwoPi;

    const double wedgeIndex = Clamp(UtdWedgeIndexFromExteriorAngle(wedge.wedge_angle_deg), 0.1, 3.0);
    const double exteriorAngle = wedgeIndex * kPi;
    const double waveNumber = kTwoPi / field.wavelength_m;
    const double s1 = std::max(1.0e-9, node.segment_length_from_previous);
    double s2 = node.diffraction_diag.s2;
    if (s2 <= 1.0e-9 && input.path && !input.path->nodes.empty()) {
        s2 = Length(Subtract(input.path->nodes.back().point, node.point));
    }
    if (s2 <= 1.0e-9) return false;
    const double distanceParameter = s1 * s2 * sinBeta * sinBeta / (s1 + s2);
    const UtdTerms terms = ComputeUtdTerms(
        waveNumber, wedgeIndex, sinBeta, phi, phiPrime, distanceParameter);

    const FaceReflection reflection0 = ResolveFaceReflection(
        zeroMaterial, std::fabs(std::sin(phiPrime)), field, input);
    const FaceReflection reflectionN = ResolveFaceReflection(
        nMaterial, std::fabs(std::sin(exteriorAngle - phi)), field, input);
    // Precise EM mode must never silently substitute PEC for a missing radio
    // material. An unresolved face invalidates this path and is visible in the
    // EM-ready path count instead of producing a plausible but wrong field.
    if (!reflection0.materialResolved || !reflectionN.materialResolved) return false;

    const Vec3 incidentEdgeBasis = SafeTransverseBasis(
        incidentDirection, Cross(incidentDirection, edgeDirection));
    const Vec3 outgoingEdgeBasis = SafeTransverseBasis(
        outgoingDirection, Scale(Cross(outgoingDirection, edgeDirection), -1.0));
    const Vec3 incidentSecondBasis = Normalize(Cross(incidentDirection, incidentEdgeBasis));
    const Vec3 outgoingSecondBasis = Normalize(Cross(outgoingDirection, outgoingEdgeBasis));
    if (Length(incidentSecondBasis) < 0.5 || Length(outgoingSecondBasis) < 0.5) return false;

    const Complex d12 = (terms.d1 + terms.d2) * -1.0;
    ComplexMatrix2 diffractionMatrix = {d12, Complex(), Complex(), d12};
    diffractionMatrix = Add(diffractionMatrix, BuildFaceTerm(
        incidentDirection, outgoingDirection, incidentEdgeBasis, outgoingEdgeBasis,
        zeroFaceNormal, reflection0, terms.d4));
    diffractionMatrix = Add(diffractionMatrix, BuildFaceTerm(
        incidentDirection, outgoingDirection, incidentEdgeBasis, outgoingEdgeBasis,
        nFaceNormal, reflectionN, terms.d3));
    diffractionMatrix = Scale(diffractionMatrix, UtdSphericalSpreadingCompensation(s1, s2));

    const ComplexVec3 incomingField = field.vector_field_valid
        ? field.electric_field_world : BuildLegacyComplexField(field);
    const Complex in0 = ComplexDot(incomingField, incidentEdgeBasis);
    const Complex in1 = ComplexDot(incomingField, incidentSecondBasis);
    const Complex out0 = diffractionMatrix.m00 * in0 + diffractionMatrix.m01 * in1;
    const Complex out1 = diffractionMatrix.m10 * in0 + diffractionMatrix.m11 * in1;
    const ComplexVec3 outgoingField = ReconstructFromBasis(
        out0, outgoingEdgeBasis, out1, outgoingSecondBasis);

    field.last_diffraction.valid = true;
    field.last_diffraction.face0_material_resolved = reflection0.materialResolved;
    field.last_diffraction.facen_material_resolved = reflectionN.materialResolved;
    field.last_diffraction.model = "finite_conductivity_utd";
    field.last_diffraction.face0_material_name = zeroMaterial;
    field.last_diffraction.facen_material_name = nMaterial;
    field.last_diffraction.face0_reflection_te = reflection0.te;
    field.last_diffraction.face0_reflection_tm = reflection0.tm;
    field.last_diffraction.facen_reflection_te = reflectionN.te;
    field.last_diffraction.facen_reflection_tm = reflectionN.tm;
    field.last_diffraction.jones_00 = diffractionMatrix.m00;
    field.last_diffraction.jones_01 = diffractionMatrix.m01;
    field.last_diffraction.jones_10 = diffractionMatrix.m10;
    field.last_diffraction.jones_11 = diffractionMatrix.m11;

    if (field.vector_field_valid) {
        field.electric_field_world = outgoingField;
        field.SyncLegacyFields();
        return true;
    }

    const double magnitude = Norm(outgoingField);
    field.amplitude_real = magnitude;
    field.amplitude_imag = 0.0;
    field.power_linear = magnitude * magnitude;
    if (magnitude > 1.0e-15) {
        const double inverse = 1.0 / magnitude;
        field.polarization_vector = MakeVec3(
            outgoingField.x.re * inverse, outgoingField.y.re * inverse, outgoingField.z.re * inverse);
        field.polarization_imag = MakeVec3(
            outgoingField.x.im * inverse, outgoingField.y.im * inverse, outgoingField.z.im * inverse);
    }
    return true;
}

} // namespace rt
