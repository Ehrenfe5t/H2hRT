// 文件目标：
// - 声明模块2批次3的场景诊断构建器。
//
// 主要功能：
// - 汇总退化面、非流形边、重复面、材质缺失等诊断项；
// - 形成 SceneDiagnostics 统一结果；
// - 为批次3报告和后续调试提供稳定入口。

#pragma once

#include "../../core/scene/Scene.h"

namespace rt {

/// <summary>
/// 生成场景诊断结果。
/// </summary>
/// <param name="scene">待分析的场景对象。</param>
/// <returns>生成好的场景诊断对象。</returns>
SceneDiagnostics BuildSceneDiagnostics(const Scene& scene);

} // namespace rt
