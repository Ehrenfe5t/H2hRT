#pragma once

#include<vector>

namespace MatrixObjectStd {

	class MatrixObject
	{
	public:
		int m = 0;
		int n = 0;
		std::vector<std::vector<double>> value;

		MatrixObject();
		~MatrixObject();
	};


	MatrixObject Init(double* a, int M, int N);
	MatrixObject Init(const std::vector<double>& a, int M, int N);
	MatrixObject Init(int N);
	/// <summary>
	/// 对角线是1其他是0的方阵
	/// </summary>
	/// <param name="N"></param>
	/// <returns></returns>
	MatrixObject InitEye(int N);

	MatrixObject Init(int M, int N);
	void print(const MatrixObject& obj);

}