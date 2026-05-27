// Declares the path pre-validation step: checks config/scene/path pointers,
// path validity, minimum node count, and transmission semantic completeness
// before the EM solver pipeline processes the path.

#pragma once

#include "EMSolverInput.h"

namespace rt {

/// <summary>
/// Validate that a path is ready for EM solver processing.
/// Checks for null config/scene/path, path validity, minimum 2 nodes, and
/// that all transmission interaction nodes carry complete semantic data
/// (valid medium_in/out IDs with different values). Sets transmission
/// completeness flags on the input for diagnostic use.
/// </summary>
/// <param name="input">Solver input bundle to validate (mutates completeness flags).</param>
/// <returns>true if path passes all checks; false if any check fails.</returns>
bool PreparePathForEM(const EMSolverInput& input);

} // namespace rt
