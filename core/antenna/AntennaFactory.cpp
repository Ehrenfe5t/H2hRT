// 文件目标：
// - 实现模块3正式最小天线工厂。
//
// 主要功能：
// - 从 AppConfig 构建 Tx/Rx Ideal 天线对象；
// - 为模块5正式消费天线输入提供最小构造入口；
// - 仅支持 A5 最小闭环所需的 Ideal 天线。

#include "AntennaFactory.h"

namespace rt {

AntennaModel BuildTxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    Vec3 polarization;
    polarization.x = 1.0;
    polarization.y = 0.0;
    polarization.z = 0.0;
    return BuildIdealAntennaModel(antennaId, config.antenna.source_type, true, config.em_solver.frequency_hz, position, polarization);
}

AntennaModel BuildRxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId)
{
    Vec3 polarization;
    polarization.x = 1.0;
    polarization.y = 0.0;
    polarization.z = 0.0;
    return BuildIdealAntennaModel(antennaId, config.antenna.source_type, false, config.em_solver.frequency_hz, position, polarization);
}

} // namespace rt
