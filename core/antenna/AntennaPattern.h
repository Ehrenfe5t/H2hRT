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

// Ludwig-3 co/cross basis with antennaUp as the boresight co-polar reference.
// Unlike the raw spherical basis, this remains independent of phi at theta=0.
inline void AntennaLudwig3BasisToWorld(const Vec3& antennaForward,
    const Vec3& antennaRight, const Vec3& antennaUp,
    double thetaRad, double phiRad,
    Vec3& coHatWorld, Vec3& crossHatWorld)
{
    Vec3 thetaHat, phiHat;
    AntennaSphericalBasisToWorld(antennaForward, antennaRight, antennaUp,
                                 thetaRad, phiRad, thetaHat, phiHat);
    const double cp = std::cos(phiRad);
    const double sp = std::sin(phiRad);
    coHatWorld = Normalize(Add(Scale(thetaHat, sp), Scale(phiHat, cp)));
    crossHatWorld = Normalize(Subtract(Scale(thetaHat, cp), Scale(phiHat, sp)));
}

struct AntennaPattern {
    // ── Gain pattern ──
    std::vector<double> thetaDeg, phiDeg, gainDBi;
    int Ntheta = 0, Nphi = 0;
    bool loaded = false;

    // v8: Per-angle polarization (Ludwig-3: pol_theta along theta_hat, pol_phi along phi_hat)
    std::vector<double> polThetaRe, polThetaIm, polPhiRe, polPhiIm;
    bool polarization_loaded = false;

    // v10.2 B7修复: 辅助函数 — 按(theta, phi)排序并验证矩形网格
    struct ThetaPhiRow { double theta, phi; int origIdx; };
    static bool BuildSortedGrid(const std::vector<double>& tT, const std::vector<double>& pP,
                                 std::vector<double>& outTheta, std::vector<double>& outPhi,
                                 int& nTheta, int& nPhi, std::vector<int>& sortedIdx)
    {
        int total = (int)tT.size();
        if (total < 4) return false;
        sortedIdx.resize(total);
        // 构建带原始索引的排序数组: 先按theta, theta相同时按phi
        std::vector<ThetaPhiRow> rows(total);
        for (int i = 0; i < total; ++i) {
            rows[i].theta = tT[i]; rows[i].phi = pP[i]; rows[i].origIdx = i;
        }
        std::sort(rows.begin(), rows.end(), [](const ThetaPhiRow& a, const ThetaPhiRow& b) {
            if (std::fabs(a.theta - b.theta) > 0.001) return a.theta < b.theta;
            return a.phi < b.phi;
        });
        // 提取唯一theta值
        outTheta.clear(); outTheta.push_back(rows[0].theta);
        for (int i = 1; i < total; ++i) {
            if (std::fabs(rows[i].theta - outTheta.back()) > 0.01)
                outTheta.push_back(rows[i].theta);
        }
        nTheta = (int)outTheta.size();
        if (nTheta < 1) return false;
        nPhi = total / nTheta;
        if (nPhi < 1 || nTheta * nPhi != total) {
            // 网格不是严格矩形: 尝试容差检测
            return false;
        }
        // 验证每个theta面的phi值一致
        outPhi.clear();
        for (int j = 0; j < nPhi; ++j) outPhi.push_back(rows[j].phi);
        // 验证phi单调性
        for (int j = 1; j < nPhi; ++j) {
            if (outPhi[j] <= outPhi[j-1]) return false;
        }
        // 验证phi覆盖[0, 360) — 至少首尾相接
        if (outPhi.front() < 0.0 || outPhi.back() > 360.0) return false;
        for (int i = 1; i < nTheta; ++i) {
            for (int j = 0; j < nPhi; ++j) {
                if (std::fabs(rows[i*nPhi + j].theta - outTheta[i]) > 0.01) return false;
                if (std::fabs(rows[i*nPhi + j].phi - outPhi[j]) > 0.01) return false;
            }
        }
        // 填充排序索引
        for (int i = 0; i < total; ++i) sortedIdx[i] = rows[i].origIdx;
        return true;
    }

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
                catch (...) { return false; }
            }
            if (row.size() >= 3) { tT.push_back(row[0]); pP.push_back(row[1]); gG.push_back(row[2]); }
        }
        if (tT.empty()) return false;

        // v10.2 B7修复: 排序+验证矩形网格 (替代旧脆弱的顺序依赖推断)
        std::vector<double> sortedPhi;
        std::vector<int> sortedIdx;
        if (!BuildSortedGrid(tT, pP, thetaDeg, sortedPhi, Ntheta, Nphi, sortedIdx)) return false;
        phiDeg = sortedPhi;
        gainDBi.resize(Ntheta * Nphi, 0.0);
        for (int sortedPos = 0; sortedPos < (int)tT.size(); ++sortedPos) {
            const int sourcePos = (sortedPos < (int)sortedIdx.size()) ? sortedIdx[sortedPos] : sortedPos;
            if (sourcePos >= 0 && sourcePos < (int)gG.size()) gainDBi[sortedPos] = gG[sourcePos];
        }
        loaded = true;
        return true;
    }

    // v8: Load per-angle complex polarization CSV.
    // Accepted formats:
    // theta,phi,PolTheta_re,PolTheta_im,PolPhi_re,PolPhi_im
    // theta,phi,Gain_dBi,PolTheta_re,PolTheta_im,PolPhi_re,PolPhi_im
    // Ludwig-3 definition: polTheta = E·theta_hat, polPhi = E·phi_hat (both complex, normalized to unity max)
    // v10.2 B7修复: 排序+验证矩形网格, 替代旧脆弱的顺序依赖推断
    bool LoadPolarizationCsv(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::vector<double> tT, pP, gG, ptR, ptI, ppR, ppI;
        bool hasEmbeddedGain = false;
        int dataColumnCount = 0;
        std::string line;
        std::getline(f, line); // skip header
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok; std::vector<double> row;
            while (std::getline(ss, tok, ',')) {
                try { row.push_back(std::stod(tok)); }
                catch (...) { return false; }
            }
            if (row.size() != 6 && row.size() != 7) return false;
            const int columns = static_cast<int>(row.size());
            if (dataColumnCount == 0) dataColumnCount = columns;
            if (columns != dataColumnCount) return false;
            hasEmbeddedGain = columns == 7;
            tT.push_back(row[0]); pP.push_back(row[1]);
            if (hasEmbeddedGain) {
                gG.push_back(row[2]);
                ptR.push_back(row[3]); ptI.push_back(row[4]);
                ppR.push_back(row[5]); ppI.push_back(row[6]);
            } else {
                ptR.push_back(row[2]); ptI.push_back(row[3]);
                ppR.push_back(row[4]); ppI.push_back(row[5]);
            }
        }
        if (tT.empty()) return false;

        // v10.2 B7修复: 排序+验证
        std::vector<double> parsedTheta;
        std::vector<double> sortedPhi;
        std::vector<int> sortedIdx;
        int nTh, nPh;
        if (!BuildSortedGrid(tT, pP, parsedTheta, sortedPhi, nTh, nPh, sortedIdx)) return false;
        if ((loaded || polarization_loaded) &&
            (Ntheta != nTh || Nphi != nPh || thetaDeg != parsedTheta || phiDeg != sortedPhi)) return false;
        Ntheta = nTh; Nphi = nPh; thetaDeg = parsedTheta; phiDeg = sortedPhi;
        int N = Ntheta * Nphi;
        if (hasEmbeddedGain && !loaded) gainDBi.resize(N, 0.0);
        polThetaRe.resize(N, 0.0); polThetaIm.resize(N, 0.0);
        polPhiRe.resize(N, 0.0); polPhiIm.resize(N, 0.0);
        for (int sortedPos = 0; sortedPos < N; ++sortedPos) {
            const int sourcePos = sortedIdx[sortedPos];
            if (hasEmbeddedGain && !loaded) gainDBi[sortedPos] = gG[sourcePos];
            polThetaRe[sortedPos] = ptR[sourcePos]; polThetaIm[sortedPos] = ptI[sourcePos];
            polPhiRe[sortedPos] = ppR[sourcePos];   polPhiIm[sortedPos] = ppI[sourcePos];
        }
        if (hasEmbeddedGain) loaded = true;
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
        // v11.2: phi periodic-interval search with wrap-gap handling.
        // When phi samples don't cover full [0,360), the gap [phiDeg.back(),360)U[0,phiDeg.front())
        // must be interpolated across the 0/360 boundary.
        int pi, pi1;
        double phiLower, phiUpper;
        if (phiDegVal < phiDeg.front()) {
            // Left periodic gap: value is between (phiDeg.back()-360) and phiDeg.front()
            pi = Nphi - 1; pi1 = 0;
            phiLower = phiDeg.back() - 360.0;
            phiUpper = phiDeg.front();
        } else if (phiDegVal >= phiDeg.back()) {
            // Right periodic gap: value is between phiDeg.back() and (phiDeg.front()+360)
            pi = Nphi - 1; pi1 = 0;
            phiLower = phiDeg.back();
            phiUpper = phiDeg.front() + 360.0;
        } else {
            pi = 0; pi1 = 1;
            for (int j = 0; j < Nphi - 1; ++j) {
                if (phiDegVal >= phiDeg[j] && phiDegVal <= phiDeg[j+1]) { pi = j; pi1 = j+1; break; }
            }
            phiLower = phiDeg[pi];
            phiUpper = phiDeg[pi1];
        }
        double pFrac = (phiUpper != phiLower)
            ? (phiDegVal - phiLower) / (phiUpper - phiLower) : 0.0;
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
        // v11.2: phi periodic-interval search with wrap-gap handling (same as QueryGainDBi).
        int pi, pi1;
        double phiLower, phiUpper;
        if (phiDegVal < phiDeg.front()) {
            pi = Nphi - 1; pi1 = 0;
            phiLower = phiDeg.back() - 360.0;
            phiUpper = phiDeg.front();
        } else if (phiDegVal >= phiDeg.back()) {
            pi = Nphi - 1; pi1 = 0;
            phiLower = phiDeg.back();
            phiUpper = phiDeg.front() + 360.0;
        } else {
            pi = 0; pi1 = 1;
            for (int j = 0; j < Nphi - 1; ++j) {
                if (phiDegVal >= phiDeg[j] && phiDegVal <= phiDeg[j+1]) { pi = j; pi1 = j+1; break; }
            }
            phiLower = phiDeg[pi];
            phiUpper = phiDeg[pi1];
        }
        double pFrac = (phiUpper != phiLower)
            ? (phiDegVal - phiLower) / (phiUpper - phiLower) : 0.0;
        auto interp = [&](const std::vector<double>& arr) {
            double v00=arr[ti*Nphi+pi], v10=arr[ti1*Nphi+pi];
            double v01=arr[ti*Nphi+pi1], v11=arr[ti1*Nphi+pi1];
            double v0=v00+tFrac*(v10-v00), v1=v01+tFrac*(v11-v01);
            return v0 + pFrac*(v1-v0);
        };
        ptR = interp(polThetaRe); ptI = interp(polThetaIm);
        ppR = interp(polPhiRe);   ppI = interp(polPhiIm);
        const double norm = std::sqrt(ptR*ptR + ptI*ptI + ppR*ppR + ppI*ppI);
        if (norm > 1e-12) {
            ptR /= norm; ptI /= norm; ppR /= norm; ppI /= norm;
        } else {
            ptR=1.0; ptI=0.0; ppR=0.0; ppI=0.0;
        }
    }
};

} // namespace rt
