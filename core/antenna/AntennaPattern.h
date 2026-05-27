#pragma once
#include "../common/math/Vec3.h"
#include "../common/math/CoordinateFrame.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace rt {

// v8: Antenna-local spherical coordinate transform.
// Antenna frame: forward(+Y boresight in pattern convention), right(+X), up(+Z).
// theta=0 -> forward direction; phi=0 -> right direction; phi measured in right-up plane.
inline void WorldToAntennaSpherical(const Vec3& worldDir,
    const Vec3& antennaForward, const Vec3& antennaRight, const Vec3& antennaUp,
    double& thetaRad, double& phiRad)
{
    Vec3 d = Normalize(worldDir);
    // Project onto antenna basis
    double df = Dot(d, antennaForward);
    double dr = Dot(d, antennaRight);
    double du = Dot(d, antennaUp);
    double clampF = std::max(-1.0, std::min(1.0, df));
    thetaRad = std::acos(clampF);  // 0 -> forward, pi -> backward
    phiRad = std::atan2(du, dr);   // measured from right toward up
    if (phiRad < 0.0) phiRad += 6.28318530717958647693;
}

// v8: Reconstruct world-space complex polarization vector from antenna-local
// Ludwig-3 components (polTheta along theta_hat, polPhi along phi_hat).
// The spherical basis vectors theta_hat, phi_hat are computed in the antenna frame.
inline void AntennaSphericalBasisToWorld(const Vec3& antennaForward,
    const Vec3& antennaRight, const Vec3& antennaUp,
    double thetaRad, double phiRad,
    Vec3& thetaHatWorld, Vec3& phiHatWorld)
{
    // theta_hat points "south" (increasing theta): away from forward, toward the horizon
    // In antenna basis: theta_hat = cos(theta)*cos(phi)*right + cos(theta)*sin(phi)*up - sin(theta)*forward
    double ct = std::cos(thetaRad), st = std::sin(thetaRad);
    double cp = std::cos(phiRad), sp = std::sin(phiRad);
    // theta_hat in world coordinates
    thetaHatWorld = MakeVec3(
        ct*cp*antennaRight.x + ct*sp*antennaUp.x - st*antennaForward.x,
        ct*cp*antennaRight.y + ct*sp*antennaUp.y - st*antennaForward.y,
        ct*cp*antennaRight.z + ct*sp*antennaUp.z - st*antennaForward.z);
    // phi_hat points "east" (increasing phi): -sin(phi)*right + cos(phi)*up
    phiHatWorld = MakeVec3(
        -sp*antennaRight.x + cp*antennaUp.x,
        -sp*antennaRight.y + cp*antennaUp.y,
        -sp*antennaRight.z + cp*antennaUp.z);
}

struct AntennaPattern {
    // ── Gain pattern ──
    std::vector<double> thetaDeg, phiDeg, gainDBi;
    int Ntheta = 0, Nphi = 0;
    bool loaded = false;

    // v8: Per-angle polarization (Ludwig-3: pol_theta along theta_hat, pol_phi along phi_hat)
    std::vector<double> polThetaRe, polThetaIm, polPhiRe, polPhiIm;
    bool polarization_loaded = false;

    bool LoadCsv(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::vector<double> tT, pP, gG;
        std::string line;
        std::getline(f, line); // skip header
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok; std::vector<double> row;
            while (std::getline(ss, tok, ',')) {
                try { row.push_back(std::stod(tok)); }
                catch (...) { row.push_back(0.0); }
            }
            if (row.size() >= 3) { tT.push_back(row[0]); pP.push_back(row[1]); gG.push_back(row[2]); }
        }
        if (tT.empty()) return false;
        for (auto& v : tT) {
            if (thetaDeg.empty() || std::fabs(v - thetaDeg.back()) > 0.01)
                thetaDeg.push_back(v);
        }
        Ntheta = (int)thetaDeg.size();
        Nphi = (int)tT.size() / Ntheta;
        if (Nphi < 1) return false;
        for (int j = 0; j < Nphi; ++j) phiDeg.push_back(pP[j]);
        gainDBi.resize(Ntheta * Nphi, 0.0);
        for (size_t i = 0; i < tT.size(); ++i) gainDBi[i] = gG[i];
        loaded = true;
        return true;
    }

    // v8: Load per-angle complex polarization CSV.
    // Format: Theta[deg],Phi[deg],Gain_dBi,PolTheta_re,PolTheta_im,PolPhi_re,PolPhi_im
    // Ludwig-3 definition: polTheta = E·theta_hat, polPhi = E·phi_hat (both complex, normalized to unity max)
    bool LoadPolarizationCsv(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::vector<double> tT, pP, gG, ptR, ptI, ppR, ppI;
        std::string line;
        std::getline(f, line); // skip header
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok; std::vector<double> row;
            while (std::getline(ss, tok, ',')) {
                try { row.push_back(std::stod(tok)); }
                catch (...) { row.push_back(0.0); }
            }
            if (row.size() >= 7) {
                tT.push_back(row[0]); pP.push_back(row[1]); gG.push_back(row[2]);
                ptR.push_back(row[3]); ptI.push_back(row[4]);
                ppR.push_back(row[5]); ppI.push_back(row[6]);
            }
        }
        if (tT.empty()) return false;
        thetaDeg.clear(); phiDeg.clear(); gainDBi.clear();
        for (auto& v : tT) {
            if (thetaDeg.empty() || std::fabs(v - thetaDeg.back()) > 0.01)
                thetaDeg.push_back(v);
        }
        Ntheta = (int)thetaDeg.size();
        Nphi = (int)tT.size() / Ntheta;
        if (Nphi < 1) return false;
        for (int j = 0; j < Nphi; ++j) phiDeg.push_back(pP[j]);
        int N = Ntheta * Nphi;
        gainDBi.resize(N, 0.0); polThetaRe.resize(N, 0.0); polThetaIm.resize(N, 0.0);
        polPhiRe.resize(N, 0.0); polPhiIm.resize(N, 0.0);
        for (size_t i = 0; i < tT.size() && (int)i < N; ++i) {
            gainDBi[i] = gG[i];
            polThetaRe[i] = ptR[i]; polThetaIm[i] = ptI[i];
            polPhiRe[i] = ppR[i];   polPhiIm[i] = ppI[i];
        }
        loaded = true;
        polarization_loaded = true;
        return true;
    }

    double QueryGainDBi(double thetaDegVal, double phiDegVal) const {
        if (!loaded || Ntheta < 1 || Nphi < 1) return 0.0;
        while (phiDegVal < 0.0) phiDegVal += 360.0;
        while (phiDegVal >= 360.0) phiDegVal -= 360.0;
        thetaDegVal = std::max(0.0, std::min(180.0, thetaDegVal));
        int ti = 0; for (int i = 0; i < Ntheta - 1; ++i)
            if (thetaDegVal >= thetaDeg[i] && thetaDegVal <= thetaDeg[i+1]) { ti = i; break; }
        int ti1 = std::min(ti + 1, Ntheta - 1);
        double tFrac = (thetaDeg[ti1] != thetaDeg[ti])
            ? (thetaDegVal - thetaDeg[ti])/(thetaDeg[ti1] - thetaDeg[ti]) : 0.0;
        int pi = 0; for (int j = 0; j < Nphi - 1; ++j)
            if (phiDegVal >= phiDeg[j] && phiDegVal <= phiDeg[j+1]) { pi = j; break; }
        int pi1 = (pi + 1) % Nphi;
        double pFrac = (phiDeg[pi1] != phiDeg[pi])
            ? (phiDegVal - phiDeg[pi])/(phiDeg[pi1] - phiDeg[pi]) : 0.0;
        double g00 = gainDBi[ti*Nphi + pi],   g10 = gainDBi[ti1*Nphi + pi];
        double g01 = gainDBi[ti*Nphi + pi1], g11 = gainDBi[ti1*Nphi + pi1];
        double g0 = g00 + tFrac*(g10 - g00), g1 = g01 + tFrac*(g11 - g01);
        return g0 + pFrac*(g1 - g0);
    }

    // v8: Query complex polarization at (theta, phi) in antenna-local spherical basis.
    // Returns (polTheta_re, polTheta_im, polPhi_re, polPhi_im) via bilinear interpolation.
    // All values are normalized such that |polTheta|^2 + |polPhi|^2 = 1 at each sampled point.
    void QueryPolarization(double thetaDegVal, double phiDegVal,
        double& ptR, double& ptI, double& ppR, double& ppI) const
    {
        ptR=1.0; ptI=0.0; ppR=0.0; ppI=0.0; // default: linear theta-polarized (vertical)
        if (!polarization_loaded) return;
        while (phiDegVal < 0.0) phiDegVal += 360.0;
        while (phiDegVal >= 360.0) phiDegVal -= 360.0;
        thetaDegVal = std::max(0.0, std::min(180.0, thetaDegVal));
        int ti = 0; for (int i = 0; i < Ntheta - 1; ++i)
            if (thetaDegVal >= thetaDeg[i] && thetaDegVal <= thetaDeg[i+1]) { ti = i; break; }
        int ti1 = std::min(ti + 1, Ntheta - 1);
        double tFrac = (thetaDeg[ti1] != thetaDeg[ti])
            ? (thetaDegVal - thetaDeg[ti])/(thetaDeg[ti1] - thetaDeg[ti]) : 0.0;
        int pi = 0; for (int j = 0; j < Nphi - 1; ++j)
            if (phiDegVal >= phiDeg[j] && phiDegVal <= phiDeg[j+1]) { pi = j; break; }
        int pi1 = (pi + 1) % Nphi;
        double pFrac = (phiDeg[pi1] != phiDeg[pi])
            ? (phiDegVal - phiDeg[pi])/(phiDeg[pi1] - phiDeg[pi]) : 0.0;
        auto interp = [&](const std::vector<double>& arr) {
            double v00=arr[ti*Nphi+pi], v10=arr[ti1*Nphi+pi];
            double v01=arr[ti*Nphi+pi1], v11=arr[ti1*Nphi+pi1];
            double v0=v00+tFrac*(v10-v00), v1=v01+tFrac*(v11-v01);
            return v0 + pFrac*(v1-v0);
        };
        ptR = interp(polThetaRe); ptI = interp(polThetaIm);
        ppR = interp(polPhiRe);   ppI = interp(polPhiIm);
    }
};

} // namespace rt
