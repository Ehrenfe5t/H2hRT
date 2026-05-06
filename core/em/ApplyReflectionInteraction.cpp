// 文件目标：
// - 实现模块5批次7的反射交互更新逻辑。
//
// 主要功能：
// - 使用第一版“几何相关”的反射系数更新复振幅；
// - 结合入射方向与法向，给出可解释的反射响应差异；
// - 为后续更严格 Fresnel 反射系数实现预留位置。

#include "ApplyReflectionInteraction.h"

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
        return Vec3{};
    }
    Vec3 result;
    result.x = v.x / len;
    result.y = v.y / len;
    result.z = v.z / len;
    return result;
}

Vec3 Reflect(const Vec3& direction, const Vec3& normal)
{
    const Vec3 n = Normalize(normal);
    const double projection = Dot(direction, n);
    Vec3 result;
    result.x = direction.x - 2.0 * projection * n.x;
    result.y = direction.y - 2.0 * projection * n.y;
    result.z = direction.z - 2.0 * projection * n.z;
    return result;
}

} // namespace

/// <summary>
/// 对反射交互更新场状态。
/// </summary>
/// <param name="field">路径级场状态累积器。</param>
/// <param name="node">当前反射节点。</param>
/// <returns>true 表示更新成功；false 表示失败。</returns>
bool ApplyReflectionInteraction(FieldAccumulator& field, const PathNode& node)
{
    if (!field.valid || !node.valid)
    {
        return false;
    }

    // A4-2 的反射升级目标：至少让反射对入射几何更敏感，而不是固定乘子。
    //
    // 这里不直接引入完整 Fresnel 公式，而是先把反射响应与：
    // 1) 入射方向和法向的夹角；
    // 2) 法向有效性；
    // 3) 反射后的极化方向变化
    // 显式挂钩，保证后续再替换为更严格模型时不需要推翻主链。

    const Vec3 incomingDirection = (Length(node.direction) > 0.0) ? Normalize(node.direction) : Vec3{0.0, 0.0, -1.0};
    const Vec3 normal = Normalize(node.surface_normal);
    const double incidenceCosine = std::fabs(Dot(incomingDirection, normal));
    const double geometryFactor = 0.35 + 0.45 * incidenceCosine;
    const double coefficient = -geometryFactor;

    field.amplitude_real *= coefficient;
    field.amplitude_imag *= coefficient;
    field.power_linear = field.amplitude_real * field.amplitude_real + field.amplitude_imag * field.amplitude_imag;
    field.phase_rad += 3.14159265358979323846;
    if (Length(field.polarization_vector) > 0.0 && Length(normal) > 0.0)
    {
        field.polarization_vector = Reflect(field.polarization_vector, normal);
    }
    else
    {
        field.polarization_vector = normal;
    }
    return true;
}

} // namespace rt
