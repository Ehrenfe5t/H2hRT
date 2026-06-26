#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
        std::getline(f, line);
        while (std::getline(f, line)) {
            if (line.empty()) continue;

            std::vector<std::string> parts;
            std::string token;
            std::istringstream ss(line);
            while (std::getline(ss, token, ',')) parts.push_back(token);
            if (parts.size() < 7) continue;

            int id = -1;
            double freq = 0.0;
            double epsR = 1.0;
            double sigma = 0.0;
            double muR = 1.0;
            std::string name;
            try {
                id = std::stoi(parts[0]);
                name = CleanMaterialName(parts[1]);
                freq = std::stod(parts[3]);
                epsR = std::stod(parts[4]);
                sigma = std::stod(parts[5]);
                muR = std::stod(parts[6]);
            } catch (...) {
                continue;
            }

            if (name.empty() || freq <= 0.0) continue;
            MaterialProps p;
            p.epsilon_r = epsR;
            p.sigma = sigma;
            p.mu_r = muR;
            p.name = name;
            byName_[name][freq] = p;
            byId_[id][freq] = p;
        }
        return !byName_.empty();
    }

    MaterialProps QueryByName(const std::string& name, double freqHz) const
    {
        auto itN = FindMaterialByName(name);
        if (itN == byName_.end()) {
            static std::unordered_set<std::string> warned;
            static std::mutex warnMutex;
            {
                std::lock_guard<std::mutex> lock(warnMutex);
                if (warned.insert(name).second) {
                    std::fprintf(stderr,
                                 "[MaterialDB] WARNING: material '%s' not found, using vacuum (epsilon_r=1.0)\n",
                                 name.c_str());
                }
            }
            return MaterialProps{};
        }
        return InterpolateOrExtrapolate(itN->second, freqHz);
    }

    MaterialProps QueryById(int id, double freqHz) const
    {
        auto it = byId_.find(id);
        if (it == byId_.end()) return MaterialProps{};
        return InterpolateOrExtrapolate(it->second, freqHz);
    }

    std::pair<double, double> QueryComplexPermittivity(
        const std::string& name,
        double freqHz,
        double freqOmega) const
    {
        MaterialProps p = QueryByName(name, freqHz);
        const double imag = (freqOmega > 0.0)
            ? p.sigma / (freqOmega * 8.8541878128e-12)
            : 0.0;
        return {p.epsilon_r, -imag};
    }

    bool empty() const { return byName_.empty(); }

    std::size_t MaterialCount() const { return byName_.size(); }

    std::vector<double> FrequencySamplesHz() const
    {
        std::vector<double> frequencies;
        for (const auto& byMaterial : byName_) {
            for (const auto& byFrequency : byMaterial.second) {
                frequencies.push_back(byFrequency.first);
            }
        }
        std::sort(frequencies.begin(), frequencies.end());
        frequencies.erase(std::unique(frequencies.begin(), frequencies.end()), frequencies.end());
        return frequencies;
    }

    bool IsFrequencyInTabulatedRange(double freqHz) const
    {
        const std::vector<double> frequencies = FrequencySamplesHz();
        if (frequencies.empty()) return false;
        return freqHz >= frequencies.front() && freqHz <= frequencies.back();
    }

    MaterialProps ExtrapolateITU(const MaterialProps& base, double baseFreqHz, double targetFreqHz) const
    {
        return ExtrapolateWithExponents(base, baseFreqHz, targetFreqHz, 0.0, 0.0);
    }

    bool HasMaterial(const std::string& name) const
    {
        return !name.empty() && FindMaterialByName(name) != byName_.end();
    }

private:
    struct ItuPowerLaw {
        const char* key;
        double a;
        double b;
        double c;
        double d;
    };

    std::unordered_map<std::string, std::map<double, MaterialProps>> byName_;
    std::unordered_map<int, std::map<double, MaterialProps>> byId_;

    using NameMapConstIterator =
        std::unordered_map<std::string, std::map<double, MaterialProps>>::const_iterator;

    NameMapConstIterator FindMaterialByName(const std::string& name) const
    {
        auto exact = byName_.find(name);
        if (exact != byName_.end()) return exact;

        const std::string key = NormalizeMaterialKey(name);
        for (auto it = byName_.begin(); it != byName_.end(); ++it) {
            if (NormalizeMaterialKey(it->first) == key) return it;
        }
        return byName_.end();
    }

    static std::string CleanMaterialName(std::string name)
    {
        const std::size_t bracket = name.find('[');
        if (bracket != std::string::npos) name = name.substr(0, bracket);
        const std::size_t paren = name.find('(');
        if (paren != std::string::npos) name = name.substr(0, paren);
        while (!name.empty() && std::isspace(static_cast<unsigned char>(name.back()))) name.pop_back();
        while (!name.empty() && std::isspace(static_cast<unsigned char>(name.front()))) name.erase(name.begin());
        return name;
    }

    static std::string NormalizeMaterialKey(const std::string& text)
    {
        std::string out;
        bool lastDash = false;
        for (unsigned char ch : text) {
            if (std::isalnum(ch)) {
                out.push_back(static_cast<char>(std::tolower(ch)));
                lastDash = false;
            } else if (!lastDash && !out.empty()) {
                out.push_back('-');
                lastDash = true;
            }
        }
        while (!out.empty() && out.back() == '-') out.pop_back();
        return out;
    }

    static const ItuPowerLaw* FindItuPowerLaw(const std::string& materialName)
    {
        static const ItuPowerLaw table[] = {
            {"vacuum", 1.0, 0.0, 0.0, 0.0},
            {"air", 1.0, 0.0, 0.0, 0.0},
            {"concrete", 5.24, 0.0, 0.0462, 0.7822},
            {"brick", 3.91, 0.0, 0.0238, 0.1600},
            {"plasterboard", 2.73, 0.0, 0.0085, 0.9395},
            {"wood", 1.99, 0.0, 0.0047, 1.0718},
            {"glass", 6.31, 0.0, 0.0036, 1.3394},
            {"ceiling-board", 1.48, 0.0, 0.0011, 1.0750},
            {"ceilingboard", 1.48, 0.0, 0.0011, 1.0750},
            {"chipboard", 2.58, 0.0, 0.0217, 0.7800},
            {"plywood", 2.71, 0.0, 0.3300, 0.0},
            {"marble", 7.074, 0.0, 0.0055, 0.9262},
            {"floorboard", 3.66, 0.0, 0.0044, 1.3515},
            {"metal", 1.0, 0.0, 1.0e7, 0.0},
            {"very-dry-ground", 3.0, 0.0, 0.00015, 2.52},
            {"medium-dry-ground", 15.0, -0.1, 0.035, 1.63},
            {"wet-ground", 30.0, -0.4, 0.15, 1.30},
        };

        const std::string key = NormalizeMaterialKey(materialName);
        for (const ItuPowerLaw& item : table) {
            const std::string itemKey = NormalizeMaterialKey(item.key);
            if (key == itemKey || key.find(itemKey) != std::string::npos) {
                return &item;
            }
        }
        return nullptr;
    }

    static double LocalPowerExponent(double f0, double y0, double f1, double y1)
    {
        if (f0 <= 0.0 || f1 <= 0.0 || y0 <= 0.0 || y1 <= 0.0 ||
            std::fabs(f1 - f0) < 1.0e-12) {
            return 0.0;
        }
        return (std::log10(y1) - std::log10(y0)) / (std::log10(f1) - std::log10(f0));
    }

    static MaterialProps ExtrapolateWithExponents(
        const MaterialProps& base,
        double baseFreqHz,
        double targetFreqHz,
        double epsilonExponent,
        double sigmaExponent)
    {
        MaterialProps r = base;
        if (baseFreqHz <= 0.0 || targetFreqHz <= 0.0) return r;

        if (const ItuPowerLaw* itu = FindItuPowerLaw(base.name)) {
            epsilonExponent = itu->b;
            sigmaExponent = itu->d;
        }

        const double ratio = targetFreqHz / baseFreqHz;
        r.epsilon_r = std::max(1.0, base.epsilon_r * std::pow(ratio, epsilonExponent));
        r.sigma = std::max(0.0, base.sigma * std::pow(ratio, sigmaExponent));
        return r;
    }

    static MaterialProps ExtrapolateOutsideRange(const std::map<double, MaterialProps>& data, double freqHz)
    {
        if (data.empty()) return MaterialProps{};
        if (data.size() == 1U) {
            return ExtrapolateWithExponents(data.begin()->second, data.begin()->first, freqHz, 0.0, 0.0);
        }

        if (freqHz < data.begin()->first) {
            auto p0 = data.begin();
            auto p1 = std::next(p0);
            const double b = LocalPowerExponent(p0->first, p0->second.epsilon_r, p1->first, p1->second.epsilon_r);
            const double d = LocalPowerExponent(p0->first, p0->second.sigma, p1->first, p1->second.sigma);
            return ExtrapolateWithExponents(p0->second, p0->first, freqHz, b, d);
        }

        auto hi = data.rbegin();
        auto lo = std::next(hi);
        const double b = LocalPowerExponent(lo->first, lo->second.epsilon_r, hi->first, hi->second.epsilon_r);
        const double d = LocalPowerExponent(lo->first, lo->second.sigma, hi->first, hi->second.sigma);
        return ExtrapolateWithExponents(hi->second, hi->first, freqHz, b, d);
    }

    static MaterialProps InterpolateOrExtrapolate(const std::map<double, MaterialProps>& data, double freqHz)
    {
        if (data.empty()) return MaterialProps{};
        if (freqHz <= 0.0) return data.begin()->second;

        auto it = data.lower_bound(freqHz);
        if (it != data.end() && std::fabs(it->first - freqHz) <= 1.0e-9) return it->second;
        if (it == data.begin() || it == data.end()) return ExtrapolateOutsideRange(data, freqHz);

        auto prev = std::prev(it);
        const double f0 = prev->first;
        const double f1 = it->first;
        const MaterialProps& p0 = prev->second;
        const MaterialProps& p1 = it->second;

        const double logF0 = std::log10(f0);
        const double logF1 = std::log10(f1);
        const double logF = std::log10(freqHz);
        const double frac = (logF - logF0) / (logF1 - logF0);

        MaterialProps r;
        r.name = p0.name;
        r.epsilon_r = std::pow(
            10.0,
            std::log10(std::max(1.0, p0.epsilon_r)) +
                frac * (std::log10(std::max(1.0, p1.epsilon_r)) -
                        std::log10(std::max(1.0, p0.epsilon_r))));
        r.sigma = std::pow(
            10.0,
            std::log10(std::max(1.0e-15, p0.sigma)) +
                frac * (std::log10(std::max(1.0e-15, p1.sigma)) -
                        std::log10(std::max(1.0e-15, p0.sigma))));
        r.mu_r = p0.mu_r + frac * (p1.mu_r - p0.mu_r);
        return r;
    }
};

} // namespace rt
