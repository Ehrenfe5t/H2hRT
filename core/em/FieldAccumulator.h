// Per-path field state accumulator that tracks complex amplitude, phase, delay,
// power, medium attenuation, and polarization through propagation and interactions.

#pragma once

#include "../path/InteractionType.h"
#include "../scene/Face.h"
#include "../common/math/ComplexVec3.h"

namespace rt {

/// <summary>
/// v9 B-6: 天线姿态结构体 — 定义天线本地坐标系
/// </summary>
struct AntennaPose {
    Point3 position;       // 天线位置
    Vec3 forward;          // 主瓣指向 (boresight)
    Vec3 up;               // "上"方向 (极化参考)
    Vec3 right;            // 右方向 (forward × up 正交化)
    bool valid = false;

    static AntennaPose Default(const Point3& pos = Point3{}) {
        AntennaPose p;
        p.position = pos;
        p.forward = MakeVec3(1.0, 0.0, 0.0);  // +X
        p.up      = MakeVec3(0.0, 0.0, 1.0);  // +Z
        p.right   = MakeVec3(0.0, -1.0, 0.0); // -Y (right-handed)
        p.valid = true;
        return p;
    }
};

/// <summary>
/// Per-path field state accumulator -- the working state that propagates through
/// free-space segments and interaction nodes before being finalized into EMPathResult.
/// v9 B1-b: 新增复矢量电场主状态, 旧标量字段降级为派生兼容输出.
/// </summary>
struct FieldAccumulator {
    double frequency_hz = 0.0;           ///< Carrier frequency used for this path.
    double wavelength_m = 0.0;           ///< Free-space wavelength = c0 / frequency_hz.
    double total_length_m = 0.0;         ///< Cumulative geometric path length (sum of all segment lengths).
    double delay_s = 0.0;                ///< Cumulative propagation delay = total_length_m / c0.
    double phase_rad = 0.0;              ///< v9 B1-b: 降级为派生兼容输出 (由electric_field_world参考分量导出).
    double amplitude_real = 1.0;         ///< v9 B1-b: 降级为派生兼容输出.
    double amplitude_imag = 0.0;         ///< v9 B1-b: 降级为派生兼容输出.
    double power_linear = 1.0;           ///< v9 B1-b: 降级为派生 = |Ex|²+|Ey|²+|Ez|².
    double free_space_amplitude_scale = 1.0;  ///< FSPL amplitude scale, applied once at FinalizeAtReceiver (not per segment).
    double free_space_power_scale = 1.0;      ///< FSPL power scale = (amplitude_scale)^2.
    double last_segment_length_m = 0.0;  ///< Length of the most recent propagation segment (used in diffraction distance parameter L).
    std::string tx_antenna_id;           ///< Transmit antenna identifier.
    std::string tx_antenna_source_type;  ///< Transmit antenna source type (e.g. "ideal", "pattern").
    std::string rx_antenna_id;           ///< Receive antenna identifier.
    std::string rx_antenna_source_type;  ///< Receive antenna source type.
    int current_medium_id = -1;          ///< ID of the medium the ray is currently traveling in (-1 = free space).
    int last_transmission_medium_in_id = -1;   ///< Medium ID on the incident side of the last transmission.
    int last_transmission_medium_out_id = -1;  ///< Medium ID on the transmitted side of the last transmission.
    double current_refractive_index = 1.0; ///< v7 H2: 当前介质折射率 n=Re(√ε_c), 自由空间=1.0.
    double current_attenuation_np_per_m = 0.0; ///< Attenuation constant (Np/m) of the current medium (0 = lossless).
    double media_attenuation_np = 0.0;   ///< Cumulative exponential attenuation in nepers = sum(alpha_i * d_i).
    bool transmission_semantic_consumed = false; ///< Whether a transmission interaction has already been applied on this path.
    Vec3 polarization_vector;            ///< v9 B1-b: 降级为派生 (由electric_field_world归一化).
    Vec3 polarization_imag;              ///< v9 B1-b: 降级为派生 (由electric_field_world归一化).
    bool valid = false;                  ///< Set to true after successful InitializeTxField; false aborts the path.

    // ── v9 B1-b: 复矢量电场主状态 ──
    ComplexVec3 electric_field_world;    ///< 世界坐标复电场矢量 [Ex,Ey,Ez].
    bool vector_field_valid = false;     ///< 复矢量场是否已初始化.

    /// <summary>
    /// v9 B1-b: 从复矢量电场派生旧标量兼容字段。
    /// 调用时机: 每次更新 electric_field_world 后 (自由空间段/交互后).
    /// 派生规则: power_linear = |E|²; amplitude = sqrt(power);
    ///           polarization = 归一化E; phase_rad = arg(主导分量).
    /// </summary>
    void SyncLegacyFields() {
        if (!vector_field_valid) return;
        double p = NormSq(electric_field_world);
        power_linear = (p > 0.0) ? p : 0.0;
        amplitude_real = std::sqrt(std::max(0.0, power_linear));
        amplitude_imag = 0.0;

        // 归一化极化矢量
        double mag = std::sqrt(p);
        if (mag > 1e-30) {
            double inv = 1.0 / mag;
            polarization_vector = MakeVec3(
                electric_field_world.x.re * inv,
                electric_field_world.y.re * inv,
                electric_field_world.z.re * inv);
            polarization_imag = MakeVec3(
                electric_field_world.x.im * inv,
                electric_field_world.y.im * inv,
                electric_field_world.z.im * inv);
        }
        // phase_rad 由主导分量决定 (兼容旧代码)
        if (std::fabs(electric_field_world.z.re) > 1e-30 || std::fabs(electric_field_world.z.im) > 1e-30)
            phase_rad = std::atan2(electric_field_world.z.im, electric_field_world.z.re);
        else if (std::fabs(electric_field_world.y.re) > 1e-30 || std::fabs(electric_field_world.y.im) > 1e-30)
            phase_rad = std::atan2(electric_field_world.y.im, electric_field_world.y.re);
        else
            phase_rad = std::atan2(electric_field_world.x.im, electric_field_world.x.re);
    }
};

} // namespace rt
