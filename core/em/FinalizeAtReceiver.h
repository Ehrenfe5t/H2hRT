// Declares the finalization step at the receiver: applies FSPL once, medium attenuation,
// Rx antenna gain, and packs the complete EMPathResult.

#pragma once

#include "EMPathResult.h"
#include "EMSolverInput.h"
#include "FieldAccumulator.h"
#include "../path/GeometricPath.h"

namespace rt {

/// <summary>
/// Finalize the accumulated field state into a per-path EM result.
/// Applies FSPL (lambda/(4*pi*d)) once over the total path length, multiplies
/// by exponential medium attenuation exp(-alpha_tot), applies Rx antenna pattern
/// gain, and packs all metadata into EMPathResult.
/// </summary>
/// <param name="field">Final field accumulator after all propagation and interactions.</param>
/// <param name="path">Geometric path providing total length, node positions, and LOS flag.</param>
/// <param name="input">Solver input for antenna model construction and Rx gain lookup.</param>
/// <returns>Complete EMPathResult with final amplitude, power, delay, phase, and metadata.</returns>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path, const EMSolverInput& input);

/// <summary>
/// Backward-compatible overload without EMSolverInput. Rx antenna gain defaults to 1.0.
/// </summary>
EMPathResult FinalizeAtReceiver(const FieldAccumulator& field, const GeometricPath& path);

} // namespace rt
