#pragma once

#include"QzQDeterminant.h"


namespace DeterminantStd {


	double CalDeterminantMatrix33Plus(double a00, double a01, double a02,
		double a10, double a11, double a12,
		double a20, double a21, double a22);

	double CalDeterminantMatrix22Plus(double a00, double a01, double a10, double a11);

	Determinant MulDoubleDeterminant(double d, const Determinant& determinant);


	/// <summary>
	/// 求伴随矩阵
	/// </summary>
	/// <param name="determinant"></param>
	/// <returns></returns>
	Determinant AdjointMatrix(const Determinant& determinant);

	/// <summary>
	/// 计算行列式的值
	/// </summary>
	/// <param name="determinant"></param>
	/// <returns></returns>
	double CalDeterminant(const Determinant& determinant);

	/// <summary>
	/// 计算行列式是否可逆，行列式的值是否不为0
	/// </summary>
	/// <param name="determinant"></param>
	/// <returns></returns>
	bool IsInvertible(const Determinant& determinant);

	/// <summary>
	/// 计算矩阵的逆矩阵
	/// </summary>
	/// <param name="determinant"></param>
	/// <param name="res"></param>
	/// <returns></returns>
	bool InverseMatrix(const Determinant& determinant, Determinant& res);

}


