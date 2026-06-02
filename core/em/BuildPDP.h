// Declares the PDP builder: converts per-path EM results into delay-vs-power
// taps for delay-spread and power-delay analysis.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build a Power Delay Profile (PDP) from per-path EM results.
/// Each valid path contributes a (delay, power) tap. Phase and complex
/// amplitude are discarded -- PDP is power-only.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>PDPResult with one tap per valid path.</returns>
PDPResult BuildPDP(const EMPathResultSet& pathResults);
// v9 step24: 带profile的重载 — 支持延迟分bin + 相干/非相干功率
PDPResult BuildPDP(const EMPathResultSet& pathResults, const EMSolveProfile& profile);

} // namespace rt
