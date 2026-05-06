// Declares the Fresnel reflection interaction: TE/TM decomposition, complex permittivity,
// Fresnel coefficients, and reflected field reconstruction at a surface hit point.

#pragma once

#include "FieldAccumulator.h"
#include "EMSolverInput.h"
#include "../path/PathNode.h"

namespace rt {

/// <summary>
/// Apply Fresnel reflection at a path interaction node.
/// Decomposes incident polarization into TE (perpendicular) and TM (parallel)
/// components, evaluates the complex Fresnel reflection coefficients using the
/// face material's complex permittivity (epsilon_r, sigma), and reconstructs
/// the reflected complex amplitude, phase, power, and polarization vector.
/// </summary>
/// <param name="field">Field accumulator to update with reflected state.</param>
/// <param name="node">Path node carrying hit point, direction, normal, and material IDs.</param>
/// <param name="input">Solver input providing the material database.</param>
/// <returns>true if reflection succeeded; false if field/node invalid or no material DB.</returns>
bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node, const EMSolverInput& input);

// Backward-compatible overload for legacy callers (no material DB)
inline bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node) {
    EMSolverInput dummy;
    return ApplyReflectionInteraction(field, node, dummy);
}

} // namespace rt
