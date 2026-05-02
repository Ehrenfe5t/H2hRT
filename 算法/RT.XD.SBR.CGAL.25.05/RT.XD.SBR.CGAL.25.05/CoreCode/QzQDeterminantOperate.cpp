
#include"QzQDeterminantOperate.h"
#include"LxQProjectDependencies.h"
#include"QzQGlobalConstant.h"
#include<iostream>
namespace DeterminantStd {

	double CalDeterminantMatrix33Plus(double a00, double a01, double a02,
		double a10, double a11, double a12,
		double a20, double a21, double a22) {
		return a00 * a11 * a22 +
			a01 * a12 * a20 +
			a02 * a10 * a21 -
			a02 * a11 * a20 -
			a00 * a12 * a21 -
			a01 * a10 * a22;
	}

	double CalDeterminantMatrix22Plus(double a00, double a01, 
		double a10, double a11) {
		return a00 * a11 - a01 * a10;
	}
	Determinant MulDoubleDeterminant(double d,const Determinant& determinant) {
		Determinant res(determinant.m, determinant.n);
		for (int i = 0; i < determinant.m; i++)
		{
			for (int j = 0; j < determinant.n; j++)
			{
				res.determinant[i][j] = d * determinant.determinant[i][j];
			}
		}
		return res;
	}


	/// <summary>
	/// 求伴随矩阵
	/// </summary>
	/// <param name="determinant"></param>
	/// <returns></returns>
	Determinant AdjointMatrix(const Determinant& determinant) {
		if (determinant.m == 3 && determinant.n == 3) {
			Determinant res(determinant.m, determinant.n);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					res.determinant[j][i] = determinant.determinant[(i + 1) % 3][(j + 1) % 3] * determinant.determinant[(i + 2) % 3][(j + 2) % 3] -
						determinant.determinant[(i + 1) % 3][(j + 2) % 3] * determinant.determinant[(i + 2) % 3][(j + 1) % 3];   // 根据伴随矩阵的计算公式计算伴随矩阵中每个元素的值
				}
			}
			return res;
		}
		else {
			ProjectDependenciesStd::DisplayPromptOrReason("未实现.", true, __FILE__, __LINE__);
			return Determinant();
		}
	}


	double CalDeterminant(const Determinant& determinant) {

		if (determinant.m == 3 && determinant.n == 3) {
			return CalDeterminantMatrix33Plus(determinant.determinant[0][0], determinant.determinant[0][1], determinant.determinant[0][2],
				determinant.determinant[1][0], determinant.determinant[1][1], determinant.determinant[1][2],
				determinant.determinant[2][0], determinant.determinant[2][1], determinant.determinant[2][2]);
		}
		else if (determinant.m == 2 && determinant.n == 2) {
			return CalDeterminantMatrix22Plus(determinant.determinant[0][0], determinant.determinant[0][1],
				determinant.determinant[1][0], determinant.determinant[1][1]);
		}
		else {

			ProjectDependenciesStd::DisplayPromptOrReason("未实现.", true, __FILE__, __LINE__);
			return 0;
		}
	}

	// 判断矩阵是否可逆
	bool IsInvertible(const Determinant& determinant) {
		return std::fabs(CalDeterminant(determinant)) > GlobalConstantStd::Eps;
	}


	bool InverseMatrix(const Determinant& determinant, Determinant& res) {
		auto det = CalDeterminant(determinant);
		if (std::fabs(det) > GlobalConstantStd::Eps) {
			if (determinant.m == 3 && determinant.n == 3) {
				auto scale = 1 / det;
				//res.m = determinant.m;
				//res.n = determinant.n;
				//res.determinant.resize(determinant.m, std::vector<double>(determinant.n));
				//res.determinant[0][0] = scale * (determinant.determinant[1][1] * determinant.determinant[2][2] - determinant.determinant[1][2] * determinant.determinant[2][1]);
				//res.determinant[0][1] = scale * (determinant.determinant[0][2] * determinant.determinant[2][1] - determinant.determinant[1][0] * determinant.determinant[2][2]);
				//res.determinant[0][2] = scale * (determinant.determinant[1][0] * determinant.determinant[1][2] - determinant.determinant[1][1] * determinant.determinant[2][0]);
				//res.determinant[1][0] = scale * (determinant.determinant[0][1] * determinant.determinant[2][2] - determinant.determinant[0][2] * determinant.determinant[1][1]);
				//res.determinant[1][1] = scale * (determinant.determinant[0][0] * determinant.determinant[2][2] - determinant.determinant[2][0] * determinant.determinant[2][1]);
				//res.determinant[1][2] = scale * (determinant.determinant[0][2] * determinant.determinant[2][0] - determinant.determinant[0][0] * determinant.determinant[1][2]);
				//res.determinant[2][0] = scale * (determinant.determinant[0][1] * determinant.determinant[1][2] - determinant.determinant[1][0] * determinant.determinant[1][1]);
				//res.determinant[2][1] = scale * (determinant.determinant[0][0] * determinant.determinant[1][0] - determinant.determinant[0][1] * determinant.determinant[0][2]);
				//res.determinant[2][2] = scale * (determinant.determinant[1][1] * determinant.determinant[0][2] - determinant.determinant[1][2] * determinant.determinant[2][0]);


				res = MulDoubleDeterminant(scale, AdjointMatrix(determinant));
				return true;
			}
			else {
				ProjectDependenciesStd::DisplayPromptOrReason("未实现.", true, __FILE__, __LINE__);
				return false;
			}
		}
		else {
			return false;
		}

	}
}