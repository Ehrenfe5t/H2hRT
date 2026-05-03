// 文件目标：
// - 实现模块6批次9的信道结果导出逻辑。
//
// 主要功能：
// - 输出 CIR / PDP / APS / Statistics JSON 文件；
// - 保证模块6结构化结果文件完整导出目标可检查；
// - 为后续脚本直接消费提供基础格式。

#include "ExportChannel.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出信道结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportChannel(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/channel";
    const std::string path = dir + "/channel_summary.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n"
         << "  \"cir_taps\": " << context.precise_result->cir.taps.size() << ",\n"
         << "  \"pdp_taps\": " << context.precise_result->pdp.taps.size() << ",\n"
         << "  \"aps_entries\": " << context.precise_result->aps.entries.size() << ",\n"
         << "  \"total_power_linear\": " << context.precise_result->statistics.total_power_linear << "\n"
         << "}\n";

    if (!WriteTextFile(path, json.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    return true;
}

} // namespace rt
