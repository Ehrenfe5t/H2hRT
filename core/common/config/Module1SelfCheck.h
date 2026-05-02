// File purpose:
// - Declare the module-1 self-check entry used to close batch 0/1 with explicit validation.
//
// Main responsibilities:
// - Re-run known negative configuration cases.
// - Ensure critical validation rules continue to block invalid startup states.
// - Provide a structured self-check result for the pipeline and logs.

#pragma once

#include "AppConfig.h"
#include "../error/RtError.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// Result of module-1 self-check execution.
/// </summary>
struct Module1SelfCheckResult {
    bool succeeded = false;
    std::vector<std::string> details;
    RtError error;
};

/// <summary>
/// Execute module-1 self-check cases.
/// </summary>
/// <param name="config">Current validated application config.</param>
/// <returns>Structured self-check result.</returns>
Module1SelfCheckResult RunModule1SelfCheck(const AppConfig& config);

} // namespace rt
