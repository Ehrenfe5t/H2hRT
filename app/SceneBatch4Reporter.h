// 文件目标：
// - 声明批次4查询门面与缓存闭环的验证输出函数。
//
// 主要功能：
// - 输出 SceneCache 命中状态、预处理模式与查询自检摘要；
// - 满足主开发文档中批次4的检测方式要求；
// - 为后续模块4调用 SceneQuery 前提供统一可观测性。

#pragma once

#include "../core/common/log/Logger.h"
#include "../preprocess/build/SceneBatch4Builder.h"

namespace rt {

/// <summary>
/// 将批次4查询门面与缓存摘要输出到日志系统。
/// </summary>
/// <param name="result">批次4构建结果。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch4Summary(const SceneBatch4BuildResult& result, Logger& logger);

} // namespace rt
