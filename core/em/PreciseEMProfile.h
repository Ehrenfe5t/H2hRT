// Declares the PreciseEM profile builder: high-fidelity mode with full complex
// field, polarization, phase, and coherent summation for detailed channel analysis.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build the PreciseEM evaluation profile.
/// Enables full complex field, polarization, and phase output with coherent
/// summation for CIR. Higher path budget (64) and no power threshold.
/// </summary>
/// <returns>EMSolveProfile configured for PreciseEM mode.</returns>
EMSolveProfile BuildPreciseEMProfile();

} // namespace rt
