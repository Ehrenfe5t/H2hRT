// Initializes the FieldAccumulator from EMSolverInput: sets frequency, wavelength,
// initial amplitude/phase, antenna IDs, polarization, and applies Tx antenna gain
// for the first ray direction.

#pragma once

#include "EMSolverInput.h"
#include "FieldAccumulator.h"

namespace rt {

/// <summary>
/// Initialize the per-path field accumulator at the transmitter.
/// Sets carrier frequency, wavelength, initial unit amplitude (1+0j), caches
/// Tx/Rx antenna metadata, copies the Tx polarization vector, and applies the
/// Tx antenna pattern gain for the initial departure direction.
/// </summary>
/// <param name="input">Solver input bundle (config, path, antennas).</param>
/// <param name="field">Field accumulator to populate (output).</param>
/// <returns>true on success; false if config/path is nullptr or frequency <= 0.</returns>
bool InitializeTxField(const EMSolverInput& input, FieldAccumulator& field);

} // namespace rt
