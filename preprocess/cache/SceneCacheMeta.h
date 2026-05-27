// 文件目标：
// - 定义模块2批次4的场景缓存元信息结构。
//
// 主要功能：
// - 记录缓存有效性判断所需的来源文件、版本与配置摘要；
// - 为 cache 命中/失效日志提供结构化依据；
// - 支撑 debug / production preprocess mode 的差异表达。

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 场景缓存元信息。
/// </summary>
struct SceneCacheMeta {
    std::string cache_format_version;
    std::string generated_timestamp;

    std::string scene_source_file_path;
    std::string scene_source_last_write_time;
    std::string scene_source_hash;

    std::string material_rule_file_path;
    std::string material_rule_last_write_time;
    std::string material_rule_hash;

    std::string material_database_file_path;
    std::string material_database_last_write_time;
    std::string material_database_hash;

    std::string preprocess_config_hash;
    std::string preprocess_algorithm_version;

    int vertex_count = 0;
    int face_count = 0;
    int edge_count = 0;
    int wedge_count = 0;

    bool contains_full_diagnostics = false;
    bool contains_debug_auxiliary_data = false;
    bool replay_support_ready = false;
    std::string cache_status_reason;
};

} // namespace rt
