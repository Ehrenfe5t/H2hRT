// Builds the Angular Power Spectrum (APS) from per-path EM results.
// v11.3: adds 2D theta-phi grid for heatmap visualization and MEG computation.

#include "BuildAPS.h"
#include "../common/config/AppConfig.h"

#include <cmath>
#include <algorithm>

namespace rt {

APSResult BuildAPS(const EMPathResultSet& pathResults)
{
    APSResult result;
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        APSEntry entry;
        entry.angle_metric = item.aoa_theta_deg;
        entry.power_linear = item.power_linear;
        result.entries.push_back(entry);
    }
    return result;
}

APSResult BuildAPS(const EMPathResultSet& pathResults, const AppConfig& config)
{
    APSResult result;

    // ── 1D entries (backward compatible) ──
    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        APSEntry entry;
        entry.angle_metric = item.aoa_theta_deg;
        entry.power_linear = item.power_linear;
        result.entries.push_back(entry);
    }

    // ── v11.3: 2D theta-phi grid ──
    const int nTheta = std::max(2, config.em_solver.aps_theta_bins);
    const int nPhi   = std::max(2, config.em_solver.aps_phi_bins);
    const double thetaMin = 0.0, thetaMax = 180.0;
    const double phiMin = 0.0, phiMax = 360.0;
    const double dTheta = (thetaMax - thetaMin) / static_cast<double>(nTheta);
    const double dPhi   = (phiMax - phiMin)   / static_cast<double>(nPhi);

    result.has_2d_grid = true;
    result.theta_min_deg = thetaMin;
    result.theta_max_deg = thetaMax;
    result.theta_step_deg = dTheta;
    result.phi_min_deg = phiMin;
    result.phi_max_deg = phiMax;
    result.phi_step_deg = dPhi;
    result.n_theta = nTheta;
    result.n_phi   = nPhi;

    const int gridSize = nTheta * nPhi;
    result.power_grid_linear.assign(gridSize, 0.0);
    result.incident_power_grid_linear.assign(gridSize, 0.0);

    for (const EMPathResult& item : pathResults.results)
    {
        if (!item.valid) continue;
        if (item.power_linear <= 0.0 && item.incident_power_linear <= 0.0) continue;

        // Bin AoA into theta-phi grid
        double t = std::max(thetaMin, std::min(thetaMax - 1e-9, item.aoa_theta_deg));
        double p = item.aoa_phi_deg;
        while (p < phiMin) p += 360.0;
        while (p >= phiMax) p -= 360.0;

        int iTheta = static_cast<int>((t - thetaMin) / dTheta);
        int iPhi   = static_cast<int>((p - phiMin) / dPhi);
        iTheta = std::max(0, std::min(nTheta - 1, iTheta));
        iPhi   = std::max(0, std::min(nPhi - 1, iPhi));

        const int gridIndex = result.GridIndex(iTheta, iPhi);
        result.power_grid_linear[gridIndex] += item.power_linear;
        result.incident_power_grid_linear[gridIndex] += item.incident_power_linear;
    }

    // ── dB grid ──
    result.power_grid_dB.resize(gridSize, -200.0);
    result.incident_power_grid_dB.resize(gridSize, -200.0);
    for (int i = 0; i < gridSize; ++i)
    {
        double p = result.power_grid_linear[i];
        result.power_grid_dB[i] = (p > 1e-30)
            ? 10.0 * std::log10(p) : -200.0;
        double pIncident = result.incident_power_grid_linear[i];
        result.incident_power_grid_dB[i] = (pIncident > 1e-30)
            ? 10.0 * std::log10(pIncident) : -200.0;
    }

    return result;
}

} // namespace rt
