// 文件目标：
// - 实现模块3正式最小天线工厂的构造逻辑。
//
// 主要功能：
// - 根据 AppConfig 构建 Tx/Rx Ideal 天线对象；
// - 在配置了 pattern_file 时自动加载天线方向图 CSV；
// - 为模块5提供统一的天线输入构造入口。

#include "AntennaFactory.h"
#include "../common/math/Vec3.h"
#include <fstream>

namespace rt {

/// <summary>
/// 构建发射端最小天线对象。
/// </summary>
/// <param name="config">统一配置对象，包含天线来源类型、频率与 pattern 路径。</param>
/// <param name="position">天线世界坐标位置。</param>
/// <param name="antennaId">天线标识字符串。</param>
/// <returns>初始化完成的发射天线模型，若配置了 pattern 文件则已加载方向图。</returns>
AntennaModel BuildTxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    // v7 H14: 极化配置优先级 — polarization_file > 默认垂直极化(Y-up)
    Vec3 pol = MakeVec3(0.0, 1.0, 0.0);
    if (!config.antenna.polarization_file.empty()) {
        std::ifstream pf(config.antenna.polarization_file);
        double px, py, pz;
        if (pf >> px >> py >> pz) { pol = Normalize(MakeVec3(px, py, pz)); }
    }
    AntennaModel m = BuildIdealAntennaModel(antennaId, config.antenna.source_type, true, config.em_solver.frequency_hz, position, pol);
    // B9: 若配置了方向图文件则加载
    if (!config.antenna.pattern_file.empty()) {
        m.pattern_file = config.antenna.pattern_file;
        m.pattern.LoadCsv(config.antenna.pattern_file);
    }
    m.polarization_file = config.antenna.polarization_file;
    return m;
}

/// <summary>
/// 构建接收端最小天线对象。
/// </summary>
/// <param name="config">统一配置对象，包含天线来源类型、频率与 pattern 路径。</param>
/// <param name="position">天线世界坐标位置。</param>
/// <param name="antennaId">天线标识字符串。</param>
/// <returns>初始化完成的接收天线模型，若配置了 pattern 文件则已加载方向图。</returns>
AntennaModel BuildRxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    // v7 H14: 极化配置优先级 — polarization_file > 默认垂直极化(Y-up)
    Vec3 pol = MakeVec3(0.0, 1.0, 0.0);
    if (!config.antenna.polarization_file.empty()) {
        std::ifstream pf(config.antenna.polarization_file);
        double px, py, pz;
        if (pf >> px >> py >> pz) { pol = Normalize(MakeVec3(px, py, pz)); }
    }
    AntennaModel m = BuildIdealAntennaModel(antennaId, config.antenna.source_type, false, config.em_solver.frequency_hz, position, pol);
    if (!config.antenna.pattern_file.empty()) {
        m.pattern_file = config.antenna.pattern_file;
        m.pattern.LoadCsv(config.antenna.pattern_file);
    }
    m.polarization_file = config.antenna.polarization_file;
    return m;
}

} // namespace rt
