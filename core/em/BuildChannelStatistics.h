// Declares the channel statistics builder: aggregates per-path EM results into
// compact metrics (path count, total/strongest power, mean delay, transmission count).

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Compute aggregate channel statistics from per-path EM results.
/// Produces valid path count, total power, strongest path power, mean delay,
/// and transmission path count for link budget and system analysis.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>ChannelStatistics with aggregate multipath metrics.</returns>
ChannelStatistics BuildChannelStatistics(const EMPathResultSet& pathResults);

} // namespace rt
