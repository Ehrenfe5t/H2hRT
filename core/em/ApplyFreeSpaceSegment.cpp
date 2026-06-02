// Per-segment free-space propagation: accumulates path length, delay, phase (k*d),
// and lossy-medium attenuation. FSPL is deferred to FinalizeAtReceiver.

#include "ApplyFreeSpaceSegment.h"
#include "../common/math/MathConstants.h"
#include "../common/math/Complex.h"
#include <cmath>

namespace rt {

/// <summary>
/// Propagates the field over a free-space segment of given length. Accumulates total path
/// length and delay, advances the phase by -k*d, and applies medium-specific attenuation
/// (Np/m) if the ray is currently inside a lossy dielectric. FSPL is deferred to finalization.
/// v9 B1-c: 若复矢量场已初始化, 对整个 electric_field_world 施加复传播因子.
/// </summary>
bool ApplyFreeSpaceSegment(FieldAccumulator& field, double segmentLengthM)
{
    if (!field.valid || segmentLengthM <= 0.0 || field.wavelength_m <= 0.0)
        return false;

    // Accumulate geometric length. Time delay and phase use the medium's refractive index.
    field.total_length_m += segmentLengthM;
    double nEff = field.current_refractive_index;
    field.delay_s += segmentLengthM * nEff / kC0;

    // ── v9 B1-c: 复矢量传播 ──
    if (field.vector_field_valid) {
        // 传播相位: exp(-j * k0 * n * d) 其中 k0 = 2π/λ
        double k0nd = kTwoPi * nEff * segmentLengthM / field.wavelength_m;
        // 介质衰减: exp(-alpha * d)
        double alpha_d = field.current_attenuation_np_per_m * segmentLengthM;
        double totalAtten = std::exp(-alpha_d);
        // 复传播因子 = totalAtten * exp(-j*k0nd) = totalAtten*(cos(k0nd) - j*sin(k0nd))
        Complex prop(totalAtten * std::cos(k0nd), -totalAtten * std::sin(k0nd));

        // E ← E * prop  (三个分量统一旋转相位 + 衰减)
        field.electric_field_world.x = field.electric_field_world.x * prop;
        field.electric_field_world.y = field.electric_field_world.y * prop;
        field.electric_field_world.z = field.electric_field_world.z * prop;

        // 派生旧标量兼容字段
        field.SyncLegacyFields();
    } else {
        // 旧标量路径: 保持兼容
        field.phase_rad -= kTwoPi * segmentLengthM * nEff / field.wavelength_m;
    }

    // Medium attenuation accumulation (for FinalizeAtReceiver mediaLoss)
    if (field.current_attenuation_np_per_m > 0.0)
        field.media_attenuation_np += field.current_attenuation_np_per_m * segmentLengthM;

    // Store last segment length for diffraction distance-parameter calculation
    field.last_segment_length_m = segmentLengthM;
    return true;
}

} // namespace rt
