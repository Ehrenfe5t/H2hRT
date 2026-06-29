// Declares the APS builder: converts per-path EM results into angle-vs-power
// entries and (v11.3) a 2D theta-phi angular power spectrum grid.

#pragma once

#include "EMProfile.h"

namespace rt {

struct AppConfig;

/// <summary>
/// Build an Angular Power Spectrum (APS) — 1D entries only (backward compatible).
/// </summary>
APSResult BuildAPS(const EMPathResultSet& pathResults);

/// <summary>
/// v11.3: Build APS with 2D theta-phi grid using config bin settings.
/// Populates aps.has_2d_grid, power_grid_linear, power_grid_dB.
/// Falls back to 1D-only if pathResults has no AoA information.
/// </summary>
APSResult BuildAPS(const EMPathResultSet& pathResults, const AppConfig& config);

} // namespace rt
