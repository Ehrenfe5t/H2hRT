// Declares the Fresnel transmission interaction through a dielectric boundary: TE/TM
// decomposition, complex permittivity, Fresnel transmission coefficients, and medium
// transition tracking for subsequent attenuation.

#pragma once

#include "FieldAccumulator.h"
#include "EMSolverInput.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// Apply Fresnel transmission through a dielectric boundary at a path node.
/// Uses the face's exit-side material properties (epsilon_r, sigma) to compute
/// TE and TM Fresnel transmission coefficients. Updates the field amplitude,
/// phase, power, and polarization, and records the new medium ID so subsequent
/// free-space segments can apply the correct medium-specific attenuation.
/// </summary>
/// <param name="field">Field accumulator to update with transmitted state and new medium ID.</param>
/// <param name="node">Path node containing transmission semantics (medium IDs, face reference).</param>
/// <param name="input">Solver input providing material database and scene geometry.</param>
/// <returns>true if transmission was applied; false if validation fails.</returns>
bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input);

// Backward-compatible overload for legacy callers (no EMSolverInput)
inline bool ApplyTransmissionInteraction(FieldAccumulator& field, const PathNode& node) {
    EMSolverInput dummy;
    return ApplyTransmissionInteraction(field, node, dummy);
}

} // namespace rt
