// 文件目标：
// - 实现模块3天线工厂的构造逻辑。
//
// 主要功能：
// - 根据 AppConfig 构建 Tx/Rx 天线对象；
// - v10.2: 支持 tx_antenna/rx_antenna 独立配置，回退到全局 antenna；
// - v10.2: 支持 per-Rx 天线覆盖 (RxTarget 扩展字段)；
// - v10.2: 增加天线姿态正交性检查 (B9修复)；
// - 在配置了 pattern_file 时自动加载天线方向图 CSV 和极化 CSV；
// - 为模块5提供统一的天线输入构造入口。

#include "AntennaFactory.h"
#include "../common/math/Vec3.h"
#include <fstream>
#include <cmath>

namespace rt {

// ── v10.2: 配置块解析 ──────────────────────────────────────────────

const AntennaConfig& ResolveTxAntennaConfig(const AppConfig& config)
{
    // 优先使用 tx_antenna (若 source_type 非空)；否则回退到全局 antenna
    if (!config.tx_antenna.source_type.empty()) return config.tx_antenna;
    return config.antenna;
}

const AntennaConfig& ResolveRxAntennaConfig(const AppConfig& config)
{
    if (!config.rx_antenna.source_type.empty()) return config.rx_antenna;
    return config.antenna;
}

// ── v10.2: 天线姿态正交性检查 (B9修复) ──────────────────────────────

static bool ValidateAntennaPose(const Vec3& fwd, const Vec3& upv)
{
    double len = Length(Cross(fwd, upv));
    // 若 forward 与 up 近乎平行 (< 0.0001 rad ≈ 0.006°)，拒绝配置
    if (len < 1e-4) {
        return false;
    }
    return true;
}

// ── v10.2: 从 AntennaConfig 块构建天线模型 (内部工具) ──────────────

AntennaModel BuildAntennaFromBlock(const AntennaConfig& antCfg, const AppConfig& config,
                                    const Point3& position, const std::string& antennaId, bool isTx)
{
    // v7 H14: 极化配置优先级 — polarization_file > 默认垂直极化(Y-up)
    Vec3 pol = MakeVec3(0.0, 1.0, 0.0);
    if (!antCfg.polarization_file.empty()) {
        std::ifstream pf(antCfg.polarization_file);
        double px, py, pz;
        if (pf >> px >> py >> pz) { pol = Normalize(MakeVec3(px, py, pz)); }
    }
    AntennaModel m = BuildIdealAntennaModel(antennaId, antCfg.source_type, isTx,
                                             config.em_solver.frequency_hz, position, pol);
    // v8: 天线姿态 — 从配置读取 forward/up, 自动计算 right
    Vec3 fwd = Normalize(MakeVec3(antCfg.forward_x, antCfg.forward_y, antCfg.forward_z));
    Vec3 upv = Normalize(MakeVec3(antCfg.up_x, antCfg.up_y, antCfg.up_z));

    // v10.2 B9修复: 检查 forward 与 up 正交性
    if (!ValidateAntennaPose(fwd, upv)) {
        // forward 与 up 平行 → right 为零向量 → 拒绝配置
        // 回退: 使用默认姿态 forward=(1,0,0), up=(0,0,1)
        fwd = MakeVec3(isTx ? 1.0 : -1.0, 0.0, 0.0);
        upv = MakeVec3(0.0, 0.0, 1.0);
    }
    m.forward = fwd;
    m.up = upv;
    m.right = Normalize(Cross(fwd, upv)); // right = forward × up

    // v8: 极化方向图加载 (优先 polarization_file, 若含 PolTheta/PolPhi 列则为逐角度极化)
    if (!antCfg.polarization_file.empty()) {
        m.polarization_file = antCfg.polarization_file;
        if (!m.pattern.LoadPolarizationCsv(antCfg.polarization_file)) {
            // v7 兼容: 若极化CSV加载失败, 回退为固定极化向量文件
            std::ifstream pf(antCfg.polarization_file);
            double px, py, pz;
            if (pf >> px >> py >> pz) { pol = Normalize(MakeVec3(px, py, pz)); }
        }
    }
    m.polarization_vector = pol;
    // 若配置了方向图文件则加载 (仅增益, 不含极化)
    if (!antCfg.pattern_file.empty()) {
        m.pattern_file = antCfg.pattern_file;
        if (!m.pattern.loaded) m.pattern.LoadCsv(antCfg.pattern_file);
    }
    return m;
}

// ── 公开构造接口 ────────────────────────────────────────────────────

AntennaModel BuildTxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    const AntennaConfig& antCfg = ResolveTxAntennaConfig(config);
    return BuildAntennaFromBlock(antCfg, config, position, antennaId, true);
}

AntennaModel BuildRxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    const AntennaConfig& antCfg = ResolveRxAntennaConfig(config);
    return BuildAntennaFromBlock(antCfg, config, position, antennaId, false);
}

// ── v10.2: per-Rx 天线覆盖 ──────────────────────────────────────────

static bool HasPerRxAntennaOverride(const RxTarget& rx)
{
    // 若 rx_source_type 非空或有 pattern/polarization 文件 → 有覆盖
    if (!rx.rx_source_type.empty()) return true;
    if (!rx.rx_pattern_file.empty() || !rx.rx_polarization_file.empty()) return true;
    // 若 forward 向量非零 (任何分量) → 有姿态覆盖
    if (std::fabs(rx.rx_forward_x) > 1e-12 || std::fabs(rx.rx_forward_y) > 1e-12 || std::fabs(rx.rx_forward_z) > 1e-12) return true;
    return false;
}

AntennaModel BuildRxAntennaModelWithOverride(const AppConfig& config, const RxTarget& rxTarget,
                                              const Point3& position, const std::string& antennaId)
{
    if (!HasPerRxAntennaOverride(rxTarget)) {
        // 无 per-Rx 覆盖 → 使用全局 rx_antenna / antenna
        return BuildRxAntennaModel(config, position, antennaId);
    }

    // 构造临时 AntennaConfig: 从 rxTarget 覆盖字段覆盖全局 rx_antenna
    const AntennaConfig& base = ResolveRxAntennaConfig(config);
    AntennaConfig antCfg = base;  // 拷贝全局默认

    if (!rxTarget.rx_source_type.empty())
        antCfg.source_type = rxTarget.rx_source_type;
    if (!rxTarget.rx_pattern_file.empty())
        antCfg.pattern_file = rxTarget.rx_pattern_file;
    if (!rxTarget.rx_polarization_file.empty())
        antCfg.polarization_file = rxTarget.rx_polarization_file;

    // 姿态覆盖: 若 forward 向量非零，覆盖
    if (std::fabs(rxTarget.rx_forward_x) > 1e-12 || std::fabs(rxTarget.rx_forward_y) > 1e-12 || std::fabs(rxTarget.rx_forward_z) > 1e-12) {
        antCfg.forward_x = rxTarget.rx_forward_x;
        antCfg.forward_y = rxTarget.rx_forward_y;
        antCfg.forward_z = rxTarget.rx_forward_z;
    }
    if (std::fabs(rxTarget.rx_up_x) > 1e-12 || std::fabs(rxTarget.rx_up_y) > 1e-12 || std::fabs(rxTarget.rx_up_z) > 1e-12) {
        antCfg.up_x = rxTarget.rx_up_x;
        antCfg.up_y = rxTarget.rx_up_y;
        antCfg.up_z = rxTarget.rx_up_z;
    }

    return BuildAntennaFromBlock(antCfg, config, position, antennaId, false);
}

} // namespace rt
