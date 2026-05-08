#pragma once

#include <string>
#include <unordered_map>
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
        if (itN == byName_.end()) return MaterialProps{};
        return Interpolate(itN->second, freqHz);
    }

    MaterialProps QueryById(int id, double freqHz) const
    {
        auto it = byId_.find(id);
        if (it == byId_.end()) return MaterialProps{};
        return Interpolate(it->second, freqHz);
    }

    bool empty() const { return byName_.empty(); }

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
        double t = (freq - f0) / (f1 - f0);
        const auto& p0 = prev->second;
        const auto& p1 = it->second;
        MaterialProps r;
        r.name = p0.name;
        r.epsilon_r = p0.epsilon_r + t * (p1.epsilon_r - p0.epsilon_r);
        r.sigma = p0.sigma + t * (p1.sigma - p0.sigma);
        r.mu_r = p0.mu_r + t * (p1.mu_r - p0.mu_r);
        return r;
    }
};

} // namespace rt
