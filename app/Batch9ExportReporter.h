// Batch-9 export, validation, and regression reporter -- declaration.
// NOTE: Transitional / legacy module. The Batch-9 self-check chain is retained alongside the
// A1 real production chain for cross-validation; it uses hand-crafted reference paths rather
// than real SearchEngine output.

#pragma once

#include "../core/common/log/Logger.h"
#include "../core/common/config/AppConfig.h"
#include "../core/scene/Scene.h"

namespace rt {

/// <summary>
/// Legacy Batch-9 export, validation, and regression self-check.
/// Builds hand-crafted reference paths, runs EM, exports all result types,
/// and logs a summary. Retained for cross-validation with the A1 real chain.
/// </summary>
/// <param name="config">Application configuration.</param>
/// <param name="scene">Static scene with closed-loop geometry.</param>
/// <param name="logger">Unified logger.</param>
/// <returns>true if the Batch-9 self-check passed; false otherwise.</returns>
bool ReportBatch9ExportSummary(const AppConfig& config, const Scene& scene, Logger& logger);

} // namespace rt
