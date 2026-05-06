// 文件目标：
// - 定义模块3正式最小天线对象与响应对象。
//
// 主要功能：
// - 提供 Ideal 天线最小可运行对象；
// - 为模块5正式消费天线输入提供统一结构；
// - 为后续 pattern / polarization / orientation / array 扩展预留正式字段位置。

#pragma once

#include "../scene/Face.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 正式最小天线对象。
/// </summary>
struct AntennaModel {
    std::string antenna_id;
    std::string source_type = "Ideal";
    bool is_tx = true;
    bool is_ideal = true;
    double frequency_hz = 0.0;
    Point3 position;
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    Vec3 polarization_vector;
    double reference_gain_linear = 1.0;
    double phase_center_offset_m = 0.0;
    std::string pattern_file;
    std::string polarization_file;
    std::string custom_metadata;
};

/// <summary>
/// 正式最小天线响应对象。
/// </summary>
struct AntennaResponse {
    bool valid = false;
    std::string antenna_id;
    std::string source_type = "Ideal";
    double gain_linear = 1.0;
    double gain_db = 0.0;
    double polarization_alignment = 1.0;
    Vec3 effective_polarization;
};

/// <summary>
/// 正式最小天线阵列对象。
/// </summary>
struct AntennaArrayModel {
    std::string array_id;
    std::vector<AntennaModel> elements;
    bool valid = false;
};

} // namespace rt
