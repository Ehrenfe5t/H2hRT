// v11.3: Cross-Polarization Ratio (XPR) statistics builder.
// Aggregates per-path XPR values into mean, median, percentiles, power-weighted mean,
// and a sorted value array for CDF plotting.

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Compute XPR statistics from per-path EM results.
/// Collects all valid XPR values (excluding inf/nan sentinels), sorts them,
/// and computes mean, median, P10, P90, power-weighted mean, min, max.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>XPRStatistics with aggregate metrics and CDF array.</returns>
XPRStatistics BuildXPRStatistics(const EMPathResultSet& pathResults);

} // namespace rt
