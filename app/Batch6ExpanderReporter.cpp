// 文件目标：
// - 实现批次6扩展器验证输出函数。
//
// 主要功能：
// - 针对反射、透射、绕射构造最小可运行自检场景；
// - 输出扩展器生成状态数与失败原因统计；
// - 为批次6正式闭环提供人工核查入口。

#include "Batch6ExpanderReporter.h"

#include "../core/search/DiffractionExpander.h"
#include "../core/search/ReflectionExpander.h"
#include "../core/search/TransmissionExpander.h"

#include <sstream>

namespace rt {

namespace {

Point3 AddOffset(const Point3& point, const Vec3& direction, double scale)
{
    Point3 value;
    value.x = point.x + direction.x * scale;
    value.y = point.y + direction.y * scale;
    value.z = point.z + direction.z * scale;
    return value;
}

void LogExpanderResult(const std::string& name, const ExpanderResult& result, Logger& logger)
{
    std::ostringstream stream;
    stream << name << ": next_states=" << result.next_states.size()
           << ", failure_reasons=" << result.failure_reasons.size();
    logger.Log(LogLevel::Info, "Module4", stream.str());
}

bool RunReflectionSelfCheck(const PathSearchContext& baseContext, Logger& logger)
{
    for (const Face& face : baseContext.scene->faces)
    {
        if (!face.reflection_enabled || face.degenerate)
        {
            continue;
        }

        PathSearchContext context = baseContext;
        context.tx_point = AddOffset(face.centroid, face.normal, 1.0);
        context.rx_point = AddOffset(face.centroid, face.normal, 1.0);

        PathState state;
        state.current_point = context.tx_point;
        state.current_direction.x = -face.normal.x;
        state.current_direction.y = -face.normal.y;
        state.current_direction.z = -face.normal.z;
        state.current_medium_id = 0;
        state.remaining_total_expansions = 1;
        state.remaining_reflections = 1;
        state.allow_reflection = true;
        state.valid = true;

        const ExpanderResult result = ExpandReflection(context, state);
        LogExpanderResult("ReflectionSelfCheck", result, logger);
        return !result.next_states.empty();
    }
    logger.Log(LogLevel::Error, "Module4", "ReflectionSelfCheck: no reflection-enabled face available.");
    return false;
}

bool RunTransmissionSelfCheck(const PathSearchContext& baseContext, Logger& logger)
{
    for (const Face& face : baseContext.scene->faces)
    {
        if (!face.transmission_enabled || !face.dual_side_material_resolved || face.degenerate)
        {
            continue;
        }

        PathSearchContext context = baseContext;
        context.tx_point = AddOffset(face.centroid, face.normal, 1.0);
        context.rx_point = AddOffset(face.centroid, face.normal, -1.0);

        PathState state;
        state.current_point = context.tx_point;
        state.current_direction.x = -face.normal.x;
        state.current_direction.y = -face.normal.y;
        state.current_direction.z = -face.normal.z;
        state.current_medium_id = 0;
        state.remaining_total_expansions = 1;
        state.remaining_transmissions = 1;
        state.allow_transmission = true;
        state.valid = true;

        const ExpanderResult result = ExpandTransmission(context, state);
        LogExpanderResult("TransmissionSelfCheck", result, logger);
        return !result.next_states.empty();
    }
    logger.Log(LogLevel::Error, "Module4", "TransmissionSelfCheck: no transmission-enabled face available.");
    return false;
}

bool RunDiffractionSelfCheck(const PathSearchContext& baseContext, Logger& logger)
{
    if (baseContext.scene->acceleration.wedge_acceleration.wedge_query_records.empty())
    {
        logger.Log(LogLevel::Error, "Module4", "DiffractionSelfCheck: no wedge query record available.");
        return false;
    }

    const WedgeQueryRecord& record = baseContext.scene->acceleration.wedge_acceleration.wedge_query_records.front();
    PathSearchContext context = baseContext;
    context.tx_point = record.center_point;
    context.rx_point = AddOffset(record.center_point, record.direction, 1.0);

    PathState state;
    state.current_point = record.center_point;
    state.current_direction = record.direction;
    state.current_medium_id = 0;
    state.remaining_total_expansions = 1;
    state.remaining_diffractions = 1;
    state.allow_diffraction = true;
    state.valid = true;

    const ExpanderResult result = ExpandDiffraction(context, state);
    LogExpanderResult("DiffractionSelfCheck", result, logger);
    return !result.next_states.empty();
}

} // namespace

/// <summary>
/// 执行批次6扩展器自检并输出摘要。
/// </summary>
/// <param name="context">批次6调试搜索上下文。</param>
/// <param name="logger">统一日志对象。</param>
/// <returns>true 表示批次6自检通过；false 表示失败。</returns>
bool ReportBatch6ExpanderSummary(const PathSearchContext& context, Logger& logger)
{
    const bool reflectionOk = RunReflectionSelfCheck(context, logger);
    const bool transmissionOk = RunTransmissionSelfCheck(context, logger);
    const bool diffractionOk = RunDiffractionSelfCheck(context, logger);

    std::ostringstream summary;
    summary << "Batch6Expanders: reflection=" << (reflectionOk ? "true" : "false")
            << ", transmission=" << (transmissionOk ? "true" : "false")
            << ", diffraction=" << (diffractionOk ? "true" : "false");
    logger.Log((reflectionOk && transmissionOk && diffractionOk) ? LogLevel::Info : LogLevel::Error,
               "Module4",
               summary.str());
    return reflectionOk && transmissionOk && diffractionOk;
}

} // namespace rt
