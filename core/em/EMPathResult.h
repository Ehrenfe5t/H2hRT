// 文件目标：
// - 定义模块5批次7的路径级电磁结果对象。
//
// 主要功能：
// - 承载单条几何路径对应的时延、相位、复振幅和功率结果；
// - 保留基础极化占位与路径引用；
// - 作为后续 CIR/PDP/APS 汇总的路径级真源。

#pragma once

#include "../path/GeometricPath.h"

#include <string>
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
    double free_space_amplitude_scale = 0.0;
    double free_space_power_scale = 0.0;
    double wavelength_m = 0.0;
    double polarization_magnitude = 0.0;
    double free_space_loss_db = 0.0;
    std::string tx_antenna_id;
    std::string tx_antenna_source_type;
    std::string rx_antenna_id;
    std::string rx_antenna_source_type;
    Vec3 polarization_vector;
    bool is_los = false;
    std::string source_path_signature;
    std::string source_tag = "unknown";
    bool contains_transmission = false;
    bool transmission_semantic_consumed = false;
    int first_transmission_medium_in_id = -1;
    int first_transmission_medium_out_id = -1;
    int last_transmission_medium_in_id = -1;
    int last_transmission_medium_out_id = -1;
};

/// <summary>
/// 路径级电磁结果集合。
/// </summary>
struct EMPathResultSet {
    std::vector<EMPathResult> results;
    bool from_search_engine = false;
    int input_path_count = 0;
    int valid_result_count = 0;
    std::string source_tag = "unknown";
};

} // namespace rt
