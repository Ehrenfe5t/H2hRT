// 文件目标：
// - 定义模块2在批次2阶段输出的场景元信息结构。
//
// 主要功能：
// - 记录场景源文件、对象数量、顶点数量、面元数量等基础统计；
// - 作为 Scene 基础语义层的概览入口；
// - 为日志、调试输出和后续验证报告提供摘要信息。

#pragma once

#include <string>

namespace rt {

/// <summary>
/// 场景元信息结构。
/// </summary>
struct SceneMeta {
    std::string source_file_path;
    std::string source_format;
    int object_count = 0;
    int vertex_count = 0;
    int normal_count = 0;
    int face_count = 0;
};

} // namespace rt
