// 文件目标：
// - 声明批次3场景拓扑、诊断与加速结构的验证输出函数。
//
// 主要功能：
// - 输出面、边、楔边、诊断与 BVH 统计；
// - 满足主开发文档中批次3的检测方式要求；
// - 为人工核查与后续回归保留统一日志出口。

#pragma once

#include "../../core/common/log/Logger.h"
#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 将批次3场景拓扑、诊断与加速摘要输出到日志系统。
/// </summary>
/// <param name="scene">待输出摘要的场景对象。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>无返回值。</returns>
void ReportSceneBatch3Summary(const Scene& scene, Logger& logger);

} // namespace rt
