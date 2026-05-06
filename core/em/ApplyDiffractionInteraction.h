// Declares the UTD (Uniform Theory of Diffraction) edge-diffraction interaction:
// Keller cone, edge-fixed coordinate frame, Kouyoumjian-Pathak diffraction coefficients
// for soft (Dirichlet) and hard (Neumann) polarizations, and diffracted field reconstruction.

#pragma once

#include "FieldAccumulator.h"
#include "EMSolverInput.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// Apply UTD edge diffraction at a wedge-diffraction path node.
/// Retrieves wedge geometry from the scene, builds the edge-fixed frame,
/// computes the Keller cone angle, evaluates soft/hard UTD diffraction
/// coefficients (Kouyoumjian-Pathak formulation with Fresnel transition),
/// and reconstructs the diffracted complex field.
/// </summary>
/// <param name="field">Field accumulator to update with diffracted state.</param>
/// <param name="node">Path node carrying wedge ID, diffraction point, and outgoing direction.</param>
/// <param name="input">Solver input providing scene (wedge geometry).</param>
/// <returns>true if diffraction was applied; false on validation/geometry failure.</returns>
bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input);

// Backward-compatible overload for legacy callers (no EMSolverInput)
inline bool ApplyDiffractionInteraction(FieldAccumulator& field, const PathNode& node) {
    EMSolverInput dummy;
    return ApplyDiffractionInteraction(field, node, dummy);
}

} // namespace rt
