// 文件目标：
// - 声明模块3正式最小天线工厂接口。
//
// 主要功能：
// - 根据 AppConfig 与几何位置构建 Tx/Rx Ideal 天线对象；
// - 为模块5正式消费天线输入提供统一构造入口；
// - 不承接复杂天线平台逻辑。

#pragma once

#include "AntennaResponse.h"
#include "../common/config/AppConfig.h"

namespace rt {

/// <summary>
/// 构建发射端最小天线对象。
/// </summary>
/// <param name="config">统一配置对象。</param>
/// <param name="position">天线位置。</param>
/// <param name="antennaId">天线标识。</param>
/// <returns>正式最小天线对象。</returns>
AntennaModel BuildTxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId);

/// <summary>
/// 构建接收端最小天线对象。
/// </summary>
/// <param name="config">统一配置对象。</param>
/// <param name="position">天线位置。</param>
/// <param name="antennaId">天线标识。</param>
/// <returns>正式最小天线对象。</returns>
AntennaModel BuildRxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId);

} // namespace rt
