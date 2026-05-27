// 文件目标：
// - 定义 3x3 矩阵及矩阵-向量运算。
//
// 主要功能：
// - 提供 Matrix3x3 类型（列主序存储）；
// - 实现矩阵-向量乘法、转置、旋转矩阵构造；
// - 提供世界坐标与局部坐标之间的变换。

#pragma once

#include "Vec3.h"

namespace rt {

/// <summary>
/// 3x3 矩阵，行主序存储（每行3元素连续），默认初始化为单位矩阵。
/// Multiply() 正确: result.x = m[0]*v.x + m[1]*v.y + m[2]*v.z
/// </summary>
struct Matrix3x3 {
    double m[9] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };

    Matrix3x3() = default;

    /// <summary>按元素构造矩阵（列主序）。</summary>
    Matrix3x3(double m00, double m01, double m02,
              double m10, double m11, double m12,
              double m20, double m21, double m22) {
        m[0] = m00; m[1] = m01; m[2] = m02;
        m[3] = m10; m[4] = m11; m[5] = m12;
        m[6] = m20; m[7] = m21; m[8] = m22;
    }

    /// <summary>由三个列向量构造矩阵。</summary>
    Matrix3x3(const Vec3& col0, const Vec3& col1, const Vec3& col2) {
        m[0] = col0.x; m[1] = col1.x; m[2] = col2.x;
        m[3] = col0.y; m[4] = col1.y; m[5] = col2.y;
        m[6] = col0.z; m[7] = col1.z; m[8] = col2.z;
    }

    /// <summary>(row, col) 索引访问元素（行主序，row*3+col）。</summary>
    double operator()(int row, int col) const { return m[row * 3 + col]; }
    /// <summary>(row, col) 可写访问。</summary>
    double& operator()(int row, int col) { return m[row * 3 + col]; }
};

/// <summary>
/// 矩阵乘以向量：M * v。
/// </summary>
/// <param name="mat">3x3 矩阵。</param>
/// <param name="v">三维列向量。</param>
/// <returns>变换后的向量。</returns>
inline Vec3 Multiply(const Matrix3x3& mat, const Vec3& v) {
    return MakeVec3(
        mat.m[0] * v.x + mat.m[1] * v.y + mat.m[2] * v.z,
        mat.m[3] * v.x + mat.m[4] * v.y + mat.m[5] * v.z,
        mat.m[6] * v.x + mat.m[7] * v.y + mat.m[8] * v.z);
}

/// <summary>
/// 矩阵转置。
/// </summary>
/// <param name="mat">原始矩阵。</param>
/// <returns>转置后的矩阵。</returns>
inline Matrix3x3 Transpose(const Matrix3x3& mat) {
    return Matrix3x3(
        mat.m[0], mat.m[3], mat.m[6],
        mat.m[1], mat.m[4], mat.m[7],
        mat.m[2], mat.m[5], mat.m[8]);
}

/// <summary>
/// 由前向、右向、上向轴构造旋转矩阵。
/// </summary>
/// <param name="forward">前向轴。</param>
/// <param name="right">右向轴。</param>
/// <param name="up">上向轴。</param>
/// <returns>列主序旋转矩阵 [right | forward | up]。</returns>
inline Matrix3x3 RotationFromAxes(const Vec3& forward, const Vec3& right, const Vec3& up) {
    return Matrix3x3(right, forward, up);
}

/// <summary>
/// 将世界空间向量变换到局部坐标。
/// </summary>
/// <param name="rot">旋转矩阵（列主序，将局部转世界）。</param>
/// <param name="worldVec">世界空间中的向量。</param>
/// <returns>局部空间中的向量。</returns>
inline Vec3 WorldToLocal(const Matrix3x3& rot, const Vec3& worldVec) {
    // 用转置矩阵（逆旋转）将世界向量转回局部空间
    return Multiply(Transpose(rot), worldVec);
}

/// <summary>
/// 将局部空间向量变换到世界坐标。
/// </summary>
/// <param name="rot">旋转矩阵（列主序）。</param>
/// <param name="localVec">局部空间中的向量。</param>
/// <returns>世界空间中的向量。</returns>
inline Vec3 LocalToWorld(const Matrix3x3& rot, const Vec3& localVec) {
    return Multiply(rot, localVec);
}

} // namespace rt
