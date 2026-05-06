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

struct AntennaPattern {
    std::vector<double> thetaDeg, phiDeg, gainDBi;
    int Ntheta = 0, Nphi = 0;
    bool loaded = false;

    bool LoadCsv(const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::vector<double> tT, pP, gG;
        std::string line;
        std::getline(f, line);
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string tok;
            std::vector<double> row;
            while (std::getline(ss, tok, ',')) {
                try { row.push_back(std::stod(tok)); }
                catch (...) { row.push_back(0.0); }
            }
            if (row.size() >= 3) { tT.push_back(row[0]); pP.push_back(row[1]); gG.push_back(row[2]); }
        }
        if (tT.empty()) return false;
        for (auto& v : tT) { if (thetaDeg.empty() || std::fabs(v - thetaDeg.back()) > 0.01) thetaDeg.push_back(v); }
        Ntheta = (int)thetaDeg.size();
        Nphi = (int)tT.size() / Ntheta;
        if (Nphi < 1) return false;
        for (int j = 0; j < Nphi; ++j) phiDeg.push_back(pP[j]);
        gainDBi.resize(Ntheta * Nphi, 0.0);
        for (size_t i = 0; i < tT.size(); ++i) gainDBi[i] = gG[i];
        loaded = true;
        return true;
    }

    double QueryGainDBi(double thetaDegVal, double phiDegVal) const {
        if (!loaded || Ntheta < 1 || Nphi < 1) return 0.0;
        while (phiDegVal < 0.0) phiDegVal += 360.0;
        while (phiDegVal >= 360.0) phiDegVal -= 360.0;
        thetaDegVal = std::max(0.0, std::min(180.0, thetaDegVal));
        int ti = 0;
        for (int i = 0; i < Ntheta - 1; ++i)
            if (thetaDegVal >= thetaDeg[i] && thetaDegVal <= thetaDeg[i+1]) { ti = i; break; }
        int ti1 = std::min(ti + 1, Ntheta - 1);
        double tFrac = (thetaDeg[ti1] != thetaDeg[ti]) ? (thetaDegVal - thetaDeg[ti])/(thetaDeg[ti1] - thetaDeg[ti]) : 0.0;
        int pi = 0;
        for (int j = 0; j < Nphi - 1; ++j)
            if (phiDegVal >= phiDeg[j] && phiDegVal <= phiDeg[j+1]) { pi = j; break; }
        int pi1 = (pi + 1) % Nphi;
        double pFrac = (phiDeg[pi1] != phiDeg[pi]) ? (phiDegVal - phiDeg[pi])/(phiDeg[pi1] - phiDeg[pi]) : 0.0;
        double g00 = gainDBi[ti*Nphi + pi], g10 = gainDBi[ti1*Nphi + pi];
        double g01 = gainDBi[ti*Nphi + pi1], g11 = gainDBi[ti1*Nphi + pi1];
        double g0 = g00 + tFrac*(g10 - g00), g1 = g01 + tFrac*(g11 - g01);
        return g0 + pFrac*(g1 - g0);
    }
};

} // namespace rt
