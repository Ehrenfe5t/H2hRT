// 文件目标：
// - 声明模块2 OBJ 导入接口。
//
// 主要功能：
// - 读取 OBJ 文本格式场景；
// - 解析对象、顶点、法向、三角面；
// - 生成批次2所需的 Scene 基础几何层对象。

#pragma once

#include "../../core/common/error/RtError.h"
#include "../../core/scene/Scene.h"

#include <string>
#include <vector>

namespace rt {

/// <summary>
/// OBJ 导入结果。
/// </summary>
struct OBJImportResult {
    bool succeeded = false;
    Scene scene;
    std::vector<RtError> errors;
};

/// <summary>
/// 从 OBJ 文本文件导入 Scene 基础几何层。
/// </summary>
/// <param name="filePath">OBJ 文件路径。</param>
/// <param name="coordinateTransform">坐标变换: "none"(默认) | "blender_z_up_to_y_up"。</param>
/// <returns>结构化 OBJ 导入结果。</returns>
OBJImportResult ImportSceneFromOBJ(const std::string& filePath,
                                   const std::string& coordinateTransform = "none");

} // namespace rt
