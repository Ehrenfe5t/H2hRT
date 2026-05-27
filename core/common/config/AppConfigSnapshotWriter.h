// 文件目标：
// - 声明 AppConfig 配置快照导出接口。
//
// 主要功能：
// - 将当前运行时 AppConfig 以 JSON 文本写入输出目录；
// - 为后续回归、复现实验与问题定位提供固定快照产物；
// - 保持与模块1统一错误体系和目录规范兼容。

#pragma once

#include "AppConfig.h"
#include "../error/RtError.h"

#include <string>

namespace rt {

/// <summary>
/// AppConfig 配置快照写出结果。
/// </summary>
struct AppConfigSnapshotWriteResult {
    bool succeeded = false;
    std::string output_file_path;
    RtError error;
};

/// <summary>
/// 将统一应用配置对象写出为 JSON 配置快照文件。
/// </summary>
/// <param name="config">待导出的统一配置对象。</param>
/// <returns>快照写出结果，包含成功标志、输出路径与错误对象。</returns>
AppConfigSnapshotWriteResult WriteAppConfigSnapshot(const AppConfig& config);

} // namespace rt
