// Declares the ISAC feature set builder: extracts sensing-relevant metrics from
// per-path EM results (earliest delay, strongest path, polarization, transmission count).

#pragma once

#include "EMProfile.h"

namespace rt {

/// <summary>
/// Build ISAC (Integrated Sensing and Communication) features from per-path EM results.
/// Extracts path count, earliest delay, strongest path power, average polarization
/// magnitude, and transmission path count for joint comms/sensing applications.
/// </summary>
/// <param name="pathResults">Per-path EM results from the solver.</param>
/// <returns>ISACFeatureSet with sensing-relevant aggregate metrics.</returns>
ISACFeatureSet BuildISACFeatureSet(const EMPathResultSet& pathResults);

} // namespace rt
