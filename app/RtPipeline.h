// Application-layer pipeline entry point.
// Owns the full startup sequence: config loading, validation, Batch0-9 check chain,
// the A1 real production chain, and the optional SBR coverage pass.

#pragma once

#include <string>

namespace rt {

/// <summary>
/// Structured result returned by one invocation of the pipeline.
/// </summary>
struct PipelineRunResult {
    /// <summary>Whether the pipeline closed-loop completed successfully.</summary>
    bool succeeded = false;

    /// <summary>Suggested process exit code; 0 = success, nonzero = failure.</summary>
    int exit_code = 1;

    /// <summary>Last batch number that completed (1-indexed).</summary>
    int completed_batch = 1;
};

/// <summary>
/// Top-level pipeline entry. Owns config load, validation, all batch closed-loops,
/// the A1 real chain, and the optional SBR pass.
/// </summary>
class RtPipeline {
public:
    /// <summary>
    /// Execute the full application startup pipeline from a JSON config file.
    /// </summary>
    /// <param name="configPath">Path to the JSON configuration file.</param>
    /// <returns>Structured result with success flag, exit code, and completed batch.</returns>
    PipelineRunResult Run(const std::string& configPath) const;
};

} // namespace rt
