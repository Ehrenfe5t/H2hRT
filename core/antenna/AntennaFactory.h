// 文件目标：
// - 声明模块3正式最小天线工厂接口。
//
// 主要功能：
// - 根据 AppConfig 与几何位置构建 Tx/Rx 天线对象；
// - v10.2: 支持 tx_antenna/rx_antenna 独立配置，回退到全局 antenna；
// - v10.2: 支持 per-Rx 天线覆盖（RxTarget扩展字段）；
// - 为模块5正式消费天线输入提供统一构造入口。

#pragma once

#include "AntennaResponse.h"
#include "../common/config/AppConfig.h"

namespace rt {

/// <summary>
/// v10.2: 解析有效的天线配置块。优先级: tx_antenna > rx_antenna > antenna (fallback)。
/// </summary>
const AntennaConfig& ResolveTxAntennaConfig(const AppConfig& config);
const AntennaConfig& ResolveRxAntennaConfig(const AppConfig& config);

/// <summary>
/// v10.2: 从 AntennaConfig 块构建天线模型 (内部工具函数)。
/// </summary>
AntennaModel BuildAntennaFromBlock(const AntennaConfig& antCfg, const AppConfig& config,
                                    const Point3& position, const std::string& antennaId, bool isTx);

/// <summary>
/// 构建发射端天线对象。
/// v10.2: 优先使用 tx_antenna 配置块，若 source_type 为空则回退到 antenna。
/// </summary>
AntennaModel BuildTxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId);

/// <summary>
/// 构建接收端天线对象。
/// v10.2: 优先使用 rx_antenna 配置块；若提供 per-Rx 覆盖字段 (RxTarget)，则临时覆盖。
/// </summary>
AntennaModel BuildRxAntennaModel(const AppConfig& config, const Point3& position, const std::string& antennaId);

/// <summary>
/// v10.2: 构建接收端天线对象，支持 per-Rx 覆盖。
/// 若 rxTarget 中天线字段非空/非零，优先使用；否则回退到全局 rx_antenna 或 antenna。
/// </summary>
AntennaModel BuildRxAntennaModelWithOverride(const AppConfig& config, const RxTarget& rxTarget,
                                              const Point3& position, const std::string& antennaId);

} // namespace rt
