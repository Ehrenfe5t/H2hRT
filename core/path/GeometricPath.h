// 文件目标：
// - 定义模块4输出给模块5的稳定几何路径结构。
//
// 主要功能：
// - 保存稳定节点序列、总长度、路径签名与基础调试信息；
// - 表达已成立的几何真实路径，而不是中间运行态；
// - 为模块5严格电磁求解提供直接输入容器。

#pragma once

#include "PathNode.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// 稳定几何路径结构。
/// </summary>
struct GeometricPath {
    int path_id = -1;
    std::vector<PathNode> nodes;
    double total_length = 0.0;
    bool is_los = false;
    std::string path_signature;
    bool valid = false;
};

/// <summary>
/// 几何路径集合结构。
/// </summary>
struct GeometricPathSet {
    std::vector<GeometricPath> paths;
};

} // namespace rt
