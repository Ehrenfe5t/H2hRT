// 文件目标：
// - 实现模块6批次9的通感结果导出逻辑。
//
// 主要功能：
// - 输出 ISACFeatureSet JSON；
// - 提供最早时延、最强路径功率等摘要；
// - 为模块6通感结果表达闭环提供基础文件。

#include "ExportISAC.h"

#include "ResultExportUtils.h"

#include <sstream>

namespace rt {

/// <summary>
/// 导出通感结果文件。
/// </summary>
/// <param name="context">结果导出上下文。</param>
/// <param name="bundle">待追加导出文件路径的总容器。</param>
/// <returns>true 表示导出成功；false 表示失败。</returns>
bool ExportISAC(const ResultExportContext& context, ExportBundle& bundle)
{
    const std::string dir = context.export_root_directory + "/isac";
    const std::string path = dir + "/isac_summary.json";
    if (!EnsureResultDirectory(dir))
    {
        return false;
    }

    std::ostringstream json;
    json << "{\n"
         << "  \"path_count\": " << context.precise_result->isac_features.path_count << ",\n"
         << "  \"earliest_delay_s\": " << context.precise_result->isac_features.earliest_delay_s << ",\n"
         << "  \"strongest_path_power_linear\": " << context.precise_result->isac_features.strongest_path_power_linear << "\n"
         << "}\n";

    if (!WriteTextFile(path, json.str()))
    {
        return false;
    }
    bundle.exported_files.push_back(path);
    return true;
}

} // namespace rt
