// Declares the CIR builder: converts per-path EM results into delay-domain taps
// with complex amplitude, power, and contributing path IDs for traceability.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build a Channel Impulse Response (CIR) from per-path EM results.
/// Each valid path above the power threshold becomes a delay-domain tap
/// carrying complex amplitude, power, and its source path ID.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <param name="profile">Profile controlling threshold and coherent flag.</param>
/// <returns>CIRResult with one tap per qualifying path.</returns>
CIRResult BuildCIR(const EMPathResultSet& pathResults, const EMSolveProfile& profile);

} // namespace rt
