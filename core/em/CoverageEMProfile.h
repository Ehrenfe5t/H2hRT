// Declares the CoverageEM profile builder: non-coherent power sum, simplified output,
// power threshold, and reduced path budget for large-scale coverage simulations.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build the CoverageEM evaluation profile.
/// Enables non-coherent power summation, discards phase/polarization to reduce
/// memory, applies a power threshold (1e-6 linear), and limits to 16 paths per Rx.
/// </summary>
/// <returns>EMSolveProfile configured for CoverageEM mode.</returns>
EMSolveProfile BuildCoverageEMProfile();

} // namespace rt
