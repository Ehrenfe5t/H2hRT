// 文件目标：
// - 实现模块4路径级签名构建逻辑。
//
// 主要功能：
// - 根据路径节点序列与总长度构建稳定签名；
// - 为批次5阶段提供最小路径级去重能力；
// - 为后续批次引入角度和对象序列增强留出统一入口。

#include "PathSignatureBuilder.h"

#include <cmath>
#include <sstream>

namespace rt {

namespace {

long long Quantize(double value, double eps)
{
    const double safeEps = (eps > 0.0) ? eps : 1.0e-6;
    return static_cast<long long>(std::llround(value / safeEps));
}

} // namespace

/// <summary>
/// 根据 GeometricPath 构建路径签名。
/// </summary>
/// <param name="path">待构建签名的几何路径。</param>
/// <param name="config">统一应用配置对象。</param>
/// <returns>稳定路径签名字符串。</returns>
std::string BuildPathSignature(const GeometricPath& path, const AppConfig& config)
{
    std::ostringstream stream;
    stream << (path.is_los ? "LOS" : "MIX") << "|" << path.nodes.size() << "|";
    for (const PathNode& node : path.nodes)
    {
        stream << static_cast<int>(node.interaction_type) << ":"
               << node.face_id << ":"
               << node.wedge_id << ":"
               << Quantize(node.point.x, config.numeric_tolerance.eps_deduplicate) << ","
               << Quantize(node.point.y, config.numeric_tolerance.eps_deduplicate) << ","
               << Quantize(node.point.z, config.numeric_tolerance.eps_deduplicate) << ";";
    }
    stream << Quantize(path.total_length, config.numeric_tolerance.eps_deduplicate);
    return stream.str();
}

} // namespace rt
