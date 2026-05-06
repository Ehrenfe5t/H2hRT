// Declares the per-segment free-space propagation step: accumulates delay, phase,
// and medium attenuation. FSPL is applied once at the receiver, not per segment.

#pragma once

#include "FieldAccumulator.h"

namespace rt {

/// <summary>
/// Propagate the field state over one free-space segment.
/// Accumulates path length, delay (d/c0), phase (-k*d), and lossy-medium
/// attenuation. FSPL is intentionally deferred to FinalizeAtReceiver.
/// </summary>
/// <param name="field">Field accumulator to update in-place.</param>
/// <param name="segmentLengthM">Length of the current propagation segment in meters.</param>
/// <returns>true if propagation succeeded; false if field is invalid or length <= 0.</returns>
bool ApplyFreeSpaceSegment(FieldAccumulator& field, double segmentLengthM);

} // namespace rt
