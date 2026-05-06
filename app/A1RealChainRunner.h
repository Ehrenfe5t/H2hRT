// A1 real production chain runner -- declaration.
// Consumes a real SearchEngineResult, drives per-path EM solving (Module 5), produces
// aggregate results (Precise + Coverage profiles), and runs the Module-6 export / validation /
// regression close-loop.

#pragma once

#include "../core/common/config/AppConfig.h"
#include "../core/common/log/Logger.h"
#include "../core/common/material/MaterialDatabase.h"
#include "../core/em/EMProfile.h"
#include "../core/result/ExportBundle.h"
#include "../core/result/RegressionReport.h"
#include "../core/result/ValidationReport.h"
#include "../core/scene/Scene.h"
#include "../core/search/SearchEngine.h"

namespace rt {

/// <summary>
/// Full result of one A1 real production chain run.
/// </summary>
struct A1RealChainRunResult {
    bool succeeded = false;
    bool used_reference_path_fallback = false;
    EMPathResultSet path_result_set;       // per-path EM results from real search paths
    EMAggregateResult precise_result;      // Precise-profile aggregate (CIR, PDP, APS, stats)
    EMAggregateResult coverage_result;     // Coverage-profile aggregate
    ExportBundle export_bundle;             // summary of all exported files
    ValidationReport validation_report;     // Module-6 validation outcome
    RegressionReport regression_report;     // Module-6 regression outcome
};

/// <summary>
/// Execute the A1 real production chain: per-path EM solve, aggregate profiles,
/// export, validation, and regression close-loop.
/// </summary>
/// <param name="config">Application configuration.</param>
/// <param name="scene">Static scene with closed-loop geometry and query facade.</param>
/// <param name="searchResult">Real SearchEngine result (Module 4 output).</param>
/// <param name="logger">Unified logger.</param>
/// <param name="materialDb">Optional material database for dielectric constants.</param>
/// <returns>Structured A1 chain result containing all EM aggregates and export artifacts.</returns>
A1RealChainRunResult RunA1RealChain(
    const AppConfig& config,
    const Scene& scene,
    const SearchEngineResult& searchResult,
    Logger& logger,
    const MaterialDatabase* materialDb = nullptr);

} // namespace rt
