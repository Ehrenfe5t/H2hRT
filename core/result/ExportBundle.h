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
    bool succeeded = false;
};

} // namespace rt
