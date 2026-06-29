// Computes Mean Effective Gain (MEG) from integrated incident-APS bin power
// and the Rx gain pattern. Solid-angle factors are already represented by
// the power accumulated in each bin and must not be applied a second time.

#include "ComputeMEG.h"
#include "../antenna/AntennaModel.h"
#include "../antenna/AntennaPattern.h"
#include "../common/math/MathConstants.h"

#include <cmath>

namespace rt {

std::pair<double, double> ComputeMEG(const APSResult& aps, const AntennaModel* rxAntenna)
{
    // ── Guard: need 2D grid ──
    if (!aps.has_2d_grid || aps.power_grid_linear.empty()) {
        return {1.0, 0.0}; // ideal reference
    }

    double numSum = 0.0;
    double denSum = 0.0;
    const std::vector<double>& incidentGrid = aps.incident_power_grid_linear.empty()
        ? aps.power_grid_linear : aps.incident_power_grid_linear;

    for (int iTheta = 0; iTheta < aps.n_theta; ++iTheta)
    {
        // θ at bin center
        double thetaDeg = aps.theta_min_deg + (iTheta + 0.5) * aps.theta_step_deg;
        double thetaRad = thetaDeg * kPi / 180.0;
        bool hasPattern = (rxAntenna && rxAntenna->pattern.loaded);

        for (int iPhi = 0; iPhi < aps.n_phi; ++iPhi)
        {
            double pwr = incidentGrid[aps.GridIndex(iTheta, iPhi)];
            if (pwr < 1e-30) continue;

            double rxGainLin = 1.0;
            if (hasPattern) {
                double phiDeg = aps.phi_min_deg + (iPhi + 0.5) * aps.phi_step_deg;
                const double phiRad = phiDeg * kPi / 180.0;
                const Vec3 worldArrivalDir = MakeVec3(
                    std::sin(thetaRad) * std::cos(phiRad),
                    std::cos(thetaRad),
                    std::sin(thetaRad) * std::sin(phiRad));
                double localThetaRad = 0.0, localPhiRad = 0.0;
                WorldToAntennaSpherical(worldArrivalDir, rxAntenna->forward,
                    rxAntenna->right, rxAntenna->up, localThetaRad, localPhiRad);
                double gainDBi = rxAntenna->pattern.QueryGainDBi(
                    localThetaRad * 180.0 / kPi, localPhiRad * 180.0 / kPi);
                rxGainLin = std::pow(10.0, gainDBi / 10.0);
            }

            numSum += rxGainLin * pwr;
            denSum += pwr;
        }
    }

    double megLinear = (denSum > 1e-30) ? (numSum / denSum) : 1.0;
    double megDB = 10.0 * std::log10(std::max(1e-30, megLinear));

    return {megLinear, megDB};
}

} // namespace rt
