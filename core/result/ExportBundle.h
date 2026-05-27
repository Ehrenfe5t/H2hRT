// 文件目标：
// - 定义模块6批次9的结构化导出结果总容器。
//
// 主要功能：
// - 汇总路径、信道、覆盖、通感、可视化与报告文件路径；
// - 作为模块6导出流程的统一返回对象；
// - 为人工核查与后续回归工具提供稳定入口。

#pragma once

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 结构化导出结果总容器。
/// </summary>
struct ExportBundle {
    std::string root_directory;
    std::vector<std::string> exported_files;
    std::string primary_input_source;
    std::string export_purpose = "analysis_friendly_output";
    std::string export_schema_version = "a8-v1";
    std::string export_view_name = "a8_handoff_view";
    int search_path_count = 0;
    int em_path_result_count = 0;
    int exported_json_file_count = 0;
    int exported_csv_file_count = 0;
    bool used_reference_path_fallback = false;
    bool succeeded = false;
};

} // namespace rt
