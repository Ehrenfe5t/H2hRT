// 文件目标：
// - 定义模块4几何寻径阶段使用的统一交互类型枚举。
//
// 主要功能：
// - 统一表达发射、接收、直达、反射、透射、绕射等节点类型；
// - 为 PathNode、PathState 与后续扩展器提供稳定类型源；
// - 避免模块4内部到处散落字符串型交互判断。

#pragma once

namespace rt {

/// <summary>
/// 几何寻径交互类型枚举。
/// </summary>
enum class InteractionType {
    None = 0,
    Tx,
    Rx,
    Los,
    Reflection,
    Transmission,
    Diffraction,
    Scattering
};

} // namespace rt
