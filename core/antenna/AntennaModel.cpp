// 文件目标：
// - 实现模块3正式最小天线对象的构建与响应评估逻辑。
//
// 主要功能：
// - 为 Ideal 天线提供统一对象构建入口；
// - 为模块5提供最小响应评估；
// - 保持后续 pattern / polarization / orientation 扩展点稳定。

#include "AntennaModel.h"

#include <cmath>

namespace rt {

namespace {

double Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double Length(const Vec3& v)
{
    return std::sqrt(Dot(v, v));
}

Vec3 Normalize(const Vec3& v)
{
    const double len = Length(v);
    if (len <= 0.0)
    {
        Vec3 fallback;
        fallback.x = 1.0;
        fallback.y = 0.0;
        fallback.z = 0.0;
        return fallback;
    }
    Vec3 result;
    result.x = v.x / len;
    result.y = v.y / len;
    result.z = v.z / len;
    return result;
}

} // namespace

/// <summary>
/// 构建正式最小 Ideal 天线对象。
/// </summary>
/// <param name="antennaId">天线标识。</param>
/// <param name="sourceType">来源类型。</param>
/// <param name="isTx">是否为发射端。</param>
/// <param name="frequencyHz">工作频率。</param>
/// <param name="position">天线位置。</param>
/// <param name="polarizationVector">极化向量。</param>
/// <returns>正式最小天线对象。</returns>
AntennaModel BuildIdealAntennaModel(
    const std::string& antennaId,
    const std::string& sourceType,
    bool isTx,
    double frequencyHz,
    const Point3& position,
    const Vec3& polarizationVector)
{
    AntennaModel model;
    model.antenna_id = antennaId;
    model.source_type = sourceType.empty() ? "Ideal" : sourceType;
    model.is_tx = isTx;
    model.is_ideal = (model.source_type == "Ideal");
    model.frequency_hz = frequencyHz;
    model.position = position;
    model.forward.x = isTx ? 1.0 : -1.0;
    model.forward.y = 0.0;
    model.forward.z = 0.0;
    model.right.x = 0.0;
    model.right.y = 1.0;
    model.right.z = 0.0;
    model.up.x = 0.0;
    model.up.y = 0.0;
    model.up.z = 1.0;
    model.polarization_vector = Normalize(polarizationVector);
    model.reference_gain_linear = 1.0;
    model.phase_center_offset_m = 0.0;
    return model;
}

/// <summary>
/// 评估最小天线响应。
/// </summary>
/// <param name="model">天线对象。</param>
/// <param name="propagationDirection">传播方向。</param>
/// <returns>正式最小天线响应对象。</returns>
AntennaResponse EvaluateAntennaResponse(const AntennaModel& model, const Vec3& propagationDirection)
{
    AntennaResponse response;
    response.valid = true;
    response.antenna_id = model.antenna_id;
    response.source_type = model.source_type;
    response.gain_linear = model.reference_gain_linear;
    response.gain_db = 10.0 * std::log10(response.gain_linear <= 0.0 ? 1.0 : response.gain_linear);
    response.effective_polarization = model.polarization_vector;

    const Vec3 dir = Normalize(propagationDirection);
    response.polarization_alignment = std::fabs(Dot(model.polarization_vector, dir));
    response.gain_linear *= (0.75 + 0.25 * response.polarization_alignment);
    response.gain_db = 10.0 * std::log10(response.gain_linear <= 0.0 ? 1.0 : response.gain_linear);
    return response;
}

} // namespace rt
