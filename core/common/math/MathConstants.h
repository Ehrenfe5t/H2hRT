// 文件目标：
// - 定义系统共享的数学与物理常量。
//
// 主要功能：
// - 提供 pi、真空光速、真空介电常数、真空磁导率、自由空间波阻抗等常量；
// - 提供数值容差常量用于几何比较与浮点判定；
// - 作为全局唯一的物理常量来源，避免散落 magic number。

#pragma once

namespace rt {

constexpr double kPi = 3.14159265358979323846;        // 圆周率
constexpr double kTwoPi = 6.28318530717958647693;     // 2 * pi
constexpr double kHalfPi = 1.57079632679489661923;    // pi / 2
constexpr double kEps = 1e-12;                        // 通用浮点容差
constexpr double kEpsLength = 1e-9;                   // 长度比较容差
constexpr double kEpsAngle = 1e-9;                    // 角度比较容差
constexpr double kC0 = 299792458.0;                   // 真空光速 (m/s)
constexpr double kEpsilon0 = 8.8541878128e-12;        // 真空介电常数 (F/m)
constexpr double kMu0 = 1.25663706212e-6;             // 真空磁导率 (H/m)
constexpr double kEta0 = 376.730313668;               // 自由空间波阻抗 (ohm)

} // namespace rt
