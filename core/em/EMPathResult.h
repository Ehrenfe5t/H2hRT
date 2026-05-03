// 文件目标：
// - 定义模块5批次7的路径级电磁结果对象。
//
// 主要功能：
// - 承载单条几何路径对应的时延、相位、复振幅和功率结果；
// - 保留基础极化占位与路径引用；
// - 作为后续 CIR/PDP/APS 汇总的路径级真源。

#pragma once

#include "../path/GeometricPath.h"

#include <vector>

namespace rt {

/// <summary>
/// 单条路径级电磁结果对象。
/// </summary>
struct EMPathResult {
    int path_id = -1;
    bool valid = false;
    double total_length_m = 0.0;
    double delay_s = 0.0;
    double phase_rad = 0.0;
    double amplitude_real = 0.0;
    double amplitude_imag = 0.0;
    double power_linear = 0.0;
    Vec3 polarization_vector;
    bool is_los = false;
};

/// <summary>
/// 路径级电磁结果集合。
/// </summary>
struct EMPathResultSet {
    std::vector<EMPathResult> results;
};

} // namespace rt
