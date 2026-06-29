// v11.3: Mean Effective Gain (MEG) computation.
// Computes MEG from the 2D APS grid and Rx antenna gain pattern.
// MEG = integral(G_rx × P_incident dOmega) / integral(P_incident dOmega).
// APS bins store integrated bin power, so the discrete sum must not apply an
// additional sin(theta) factor.

#pragma once

#include "EMProfile.h"

namespace rt {

struct AntennaModel;

/// <summary>
/// Compute Mean Effective Gain (MEG) from the 2D APS grid and Rx antenna pattern.
/// Sets meg_linear and meg_dB in the output struct.
/// If rxAntenna is null or has no pattern, MEG = 1.0 (0 dB) — ideal isotropic reference.
/// </summary>
/// <param name="aps">APS result with populated 2D grid.</param>
/// <param name="rxAntenna">Rx antenna model (may be null for ideal).</param>
/// <returns>Pair of {meg_linear, meg_dB}.</returns>
std::pair<double, double> ComputeMEG(const APSResult& aps, const AntennaModel* rxAntenna);

} // namespace rt
