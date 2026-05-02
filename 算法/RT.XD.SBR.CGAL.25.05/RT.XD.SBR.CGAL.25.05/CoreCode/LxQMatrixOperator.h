#pragma once

#include"LxQMatrixObject.h"

namespace MatrixOperatorStd {

	/// <summary>
	/// 矩阵转置
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	MatrixObjectStd::MatrixObject Trans(const MatrixObjectStd::MatrixObject& a);

	/// <summary>
	/// 两个矩阵相乘
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns></returns>
	MatrixObjectStd::MatrixObject Multiply(const MatrixObjectStd::MatrixObject& a,
		const MatrixObjectStd::MatrixObject& b);

	/// <summary>
	/// 求矩阵的秩
	/// </summary>
	/// <param name="a0"></param>
	/// <returns></returns>
	int Rank(const MatrixObjectStd::MatrixObject& a0);

	/// <summary>
	/// 方阵求逆
	/// </summary>
	/// <param name="a0"></param>
	/// <returns></returns>
	MatrixObjectStd::MatrixObject inv(const MatrixObjectStd::MatrixObject& a0);

	bool inv_safe(const MatrixObjectStd::MatrixObject& a0, MatrixObjectStd::MatrixObject& c);
	/// <summary>
	/// 矩阵求广义逆
	/// </summary>
	/// <param name="a0"></param>
	/// <returns></returns>
	MatrixObjectStd::MatrixObject pinv(const MatrixObjectStd::MatrixObject& a0);

	bool pinv_safe(const MatrixObjectStd::MatrixObject& a0, MatrixObjectStd::MatrixObject& result);

}