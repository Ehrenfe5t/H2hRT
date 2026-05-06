// 文件目标：
// - 定义模块5批次7的路径级电磁求解输入结构。
//
// 主要功能：
// - 汇总单条 GeometricPath、电磁配置与场景引用；
// - 作为 PreciseEM 主链的统一输入容器；
// - 避免模块5各子过程重复散落参数列表。

#pragma once

#include "../common/config/AppConfig.h"
#include "../antenna\AntennaModel.h"
#include "../path/GeometricPath.h"
#include "../scene/Scene.h"

namespace rt {

/// <summary>
/// 路径级电磁求解输入结构。
/// </summary>
struct EMSolverInput {
    const AppConfig* config = nullptr;
    const Scene* scene = nullptr;
    const GeometricPath* path = nullptr;
    const AntennaModel* tx_antenna = nullptr;
    const AntennaModel* rx_antenna = nullptr;
    bool transmission_semantic_complete = true;
    int first_transmission_medium_in_id = -1;
    int first_transmission_medium_out_id = -1;
};

} // namespace rt
