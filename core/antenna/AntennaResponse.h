// 文件目标：
// - 声明模块3正式最小天线构建与响应评估接口。
//
// 主要功能：
// - 提供 Ideal 天线最小构建入口；
// - 提供最小响应评估入口；
// - 为模块5正式消费天线输入提供稳定接口。

#pragma once

#include "AntennaModel.h"

namespace rt {

/// <summary>
/// 构建正式最小 Ideal 天线对象。
/// </summary>
/// <param name="antennaId">天线标识。</param>
/// <param name="sourceType">来源类型。</param>
/// <param name="isTx">是否为发射端。</param>
/// <param name="frequencyHz">工作频率。</param>
/// <param name="position">天线位置。</param>
/// <param name="polarizationVector">极化向量。</param>
/// <returns>正式最小天线对象。</returns>
AntennaModel BuildIdealAntennaModel(
    const std::string& antennaId,
    const std::string& sourceType,
    bool isTx,
    double frequencyHz,
    const Point3& position,
    const Vec3& polarizationVector);

/// <summary>
/// 评估最小天线响应。
/// </summary>
/// <param name="model">天线对象。</param>
/// <param name="propagationDirection">传播方向。</param>
/// <returns>正式最小天线响应对象。</returns>
AntennaResponse EvaluateAntennaResponse(const AntennaModel& model, const Vec3& propagationDirection);

} // namespace rt
