// 文件目标：
// - 声明批次2场景语义层验证输出函数。
//
// 主要功能：
// - 输出 SceneMeta 摘要；
// - 输出对象级材质绑定记录；
// - 输出面元 front/back 材质摘要；
// - 为批次2验证方式提供统一日志出口。

#pragma once

#include "../core/scene/Scene.h"
#include "../core/common/log/Logger.h"

namespace rt {

/// <summary>
/// 将批次2场景语义层摘要输出到日志系统。
/// </summary>
/// <param name="scene">待输出摘要的场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch2Summary(const Scene& scene, Logger& logger);

} // namespace rt
