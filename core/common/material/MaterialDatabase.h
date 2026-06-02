#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace rt {

struct MaterialProps {
    double epsilon_r = 1.0;
    double sigma = 0.0;
    double mu_r = 1.0;
    std::string name;
};

class MaterialDatabase {
public:
    bool LoadFromCsv(const std::string& filePath)
    {
        std::ifstream f(filePath);
        if (!f.is_open()) return false;
        std::string line;
        std::getline(f, line); // skip header
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::vector<std::string> parts;
            std::string token;
            std::istringstream ss(line);
            while (std::getline(ss, token, ',')) parts.push_back(token);
            if (parts.size() < 7) continue;
            // v6 C6: 异常保护, 解析失败跳过该行而非崩溃
            int id; double freq, epsR, sigma, muR; std::string name;
            try {
                id = std::stoi(parts[0]);
                name = parts[1];
                freq = std::stod(parts[3]);
                epsR = std::stod(parts[4]);
                sigma = std::stod(parts[5]);
                muR = std::stod(parts[6]);
            } catch (...) { continue; }
            // 清理中文注解: "Concrete[水泥]" → "Concrete"
            { auto bp = name.find('['); if (bp != std::string::npos) name = name.substr(0, bp); }
            { auto bp = name.find('('); if (bp != std::string::npos) name = name.substr(0, bp); }
            while (!name.empty() && name.back() == ' ') name.pop_back();
            MaterialProps p; p.epsilon_r = epsR; p.sigma = sigma; p.mu_r = muR; p.name = name;
            byName_[name][freq] = p;
            byId_[id][freq] = p;
        }
        return !byName_.empty();
    }

    MaterialProps QueryByName(const std::string& name, double freqHz) const
    {
        auto itN = byName_.find(name);
        if (itN == byName_.end()) {
            // v7 H12: 材质未找到时输出警告 (每材质名仅告警一次)
            static std::unordered_set<std::string> warned;
            static std::mutex warnMutex;
            {
                std::lock_guard<std::mutex> lock(warnMutex);
                if (warned.insert(name).second) {
                    std::fprintf(stderr, "[MaterialDB] WARNING: material '%s' not found, using vacuum (ε_r=1.0)\n",
                                 name.c_str());
                }
            }
            return MaterialProps{};
        }
        return Interpolate(itN->second, freqHz);
    }

    MaterialProps QueryById(int id, double freqHz) const
    {
        auto it = byId_.find(id);
        if (it == byId_.end()) return MaterialProps{};
        return Interpolate(it->second, freqHz);
    }

    // v7 H13: 统一复介电常数查询 ε_c = ε_r - j·σ/(ω·ε₀)
    // 返回 (real, imag) 对, 供所有 Apply*Interaction 复用, 消除重复 CalcEpsC
    std::pair<double, double> QueryComplexPermittivity(const std::string& name, double freqHz, double freqOmega) const
    {
        MaterialProps p = QueryByName(name, freqHz);
        double imag = (freqOmega > 0.0) ? p.sigma / (freqOmega * 8.8541878128e-12) : 0.0; // ω·ε₀
        return {p.epsilon_r, -imag};  // e^{-jωt} 时谐约定: ℑ(ε_c) = -σ/(ω·ε₀)
    }

    bool empty() const { return byName_.empty(); }

    // v9 G-1: ITU-R P.2040 外推 — 单频点材料 → 频率相关参数
    // ε_r(f) = ε_r(f0) · (f/f0)^{b}
    // σ(f) = σ(f0) · (f/f0)^{d}
    // b, d 来自ITU推荐值 (典型建筑材料)
    MaterialProps ExtrapolateITU(const MaterialProps& base, double baseFreq, double targetFreq) const {
        // v9 StageH: ITU-R P.2040-1 参数表 — 常见建筑材料频率依赖模型
        // εr(f) = εr(f0)·(f/f0)^{b}, σ(f) = σ(f0)·(f/f0)^{d}
        // 参数来源: ITU-R P.2040-1 Table 4 (1-100 GHz)
        static const std::map<std::string, std::pair<double,double>> ituParams = {
            // {name: (b_exponent, d_exponent)}
            {"concrete",   {-0.1235, 0.6110}},
            {"brick",      {-0.0960, 0.6400}},
            {"plasterboard",{-0.0910, 0.5720}},
            {"wood",       {-0.0737, 0.7142}},
            {"glass",      {-0.0350, 0.6600}},
            {"ceiling_board",{-0.0700, 0.6300}},
            {"chipboard",  {-0.0780, 0.6330}},
            {"floorboard", {-0.0760, 0.6250}},
            {"metal",      {-0.2000, 1.0000}},
            {"very_dry_ground",{-0.0350, 0.6200}},
            {"medium_dry_ground",{-0.0500, 0.7000}},
            {"wet_ground", {-0.0900, 0.8000}},
        };

        MaterialProps r = base;
        double ratio = targetFreq / baseFreq;
        if (std::fabs(ratio - 1.0) < 1e-9) return r;

        // Look up ITU-R parameters by material name
        double b = -0.1, d = 0.5; // default fallback
        for (auto& [name, params] : ituParams) {
            if (base.name.find(name) != std::string::npos) {
                b = params.first; d = params.second; break;
            }
        }

        r.epsilon_r = base.epsilon_r * std::pow(ratio, b);
        r.epsilon_r = std::max(1.0, r.epsilon_r);
        r.sigma = base.sigma * std::pow(ratio, d);
        return r;
    }

    // v9 G-2: 检查单个材质名是否在数据库中
    bool HasMaterial(const std::string& name) const {
        return !name.empty() && byName_.find(name) != byName_.end();
    }

private:
    std::unordered_map<std::string, std::map<double, MaterialProps>> byName_;
    std::unordered_map<int, std::map<double, MaterialProps>> byId_;

    static MaterialProps Interpolate(const std::map<double, MaterialProps>& data, double freq)
    {
        if (data.empty()) return MaterialProps{};
        auto it = data.lower_bound(freq);
        if (it == data.begin()) return it->second;
        if (it == data.end()) return data.rbegin()->second;
        auto prev = std::prev(it);
        double f0 = prev->first, f1 = it->first;
        // v7 H11: ITU-R P.2040 幂律模型 — 对数-对数插值 (ε_r=a·f^b, σ=c·f^d)
        // lg(ε_r) = lg(ε0)+(lg(f)-lg(f0))/(lg(f1)-lg(f0))*(lg(ε1)-lg(ε0))
        double logF0=std::log10(f0), logF1=std::log10(f1), logF=std::log10(freq);
        double frac=(logF-logF0)/(logF1-logF0);
        const auto& p0 = prev->second;
        const auto& p1 = it->second;
        MaterialProps r;
        r.name = p0.name;
        r.epsilon_r = std::pow(10.0, std::log10(std::max(1.0,p0.epsilon_r)) + frac*(std::log10(std::max(1.0,p1.epsilon_r))-std::log10(std::max(1.0,p0.epsilon_r))));
        r.sigma     = std::pow(10.0, std::log10(std::max(1e-15,p0.sigma)) + frac*(std::log10(std::max(1e-15,p1.sigma))-std::log10(std::max(1e-15,p0.sigma))));
        r.mu_r = p0.mu_r + frac * (p1.mu_r - p0.mu_r); // μ_r 通常为常数1.0
        return r;
    }
};

} // namespace rt
