


#include"LxQMatrixObject.h"
#include<iostream>

namespace MatrixObjectStd {

	MatrixObject::MatrixObject()
	{
		this->m = 0;
		this->n = 0;
	}

	MatrixObject::~MatrixObject()
	{
	}

	MatrixObject Init(double* a, int M, int N)
	{
		MatrixObject res;
		if (M > 0 && N > 0)
		{
			res.m = M;
			res.n = N;
			res.value.clear();
			res.value.resize(res.m, std::vector<double>(res.n));
			for (int i = 0; i < res.m; i++)
			{
				for (int j = 0; j < res.n; j++)
				{
					res.value[i][j] = *(a + i * res.n + j);
				}
			}
		}
		return res;
	}

	MatrixObject Init(const std::vector<double>& a, int M, int N)
	{
		MatrixObject res;
		if (M > 0 && N > 0)
		{
			res.m = M;
			res.n = N;
			res.value.clear();
			res.value.resize(res.m, std::vector<double>(res.n));
			for (int i = 0; i < res.m; i++)
			{
				for (int j = 0; j < res.n; j++)
				{
					res.value[i][j] = a[i * res.n + j]; 
				}
			}
		}
		return res;
	}

	MatrixObject Init(int N)
	{
		MatrixObject res;
		if (N > 0)
		{
			res.m = N;
			res.n = N;

			res.value.clear();
			res.value.resize(res.m, std::vector<double>(res.n));
			for (int i = 0; i < res.m; i++)
			{
				for (int j = 0; j < res.n; j++)
				{
					res.value[i][j] = 0;
				}
			}
		}
		return res;
	}
	MatrixObject InitEye(int N)
	{
		MatrixObject res;
		if (N > 0)
		{
			res.m = N;
			res.n = N;

			res.value.clear();
			res.value.resize(res.m, std::vector<double>(res.n));
			for (int i = 0; i < res.m; i++)
			{
				for (int j = 0; j < res.n; j++)
				{
					if (i == j)
						res.value[i][j] = 1;
					else
						res.value[i][j] = 0;
				}
			}
		}
		return res;
	}
	MatrixObject Init(int M, int N)
	{
		MatrixObject res;
		if (M > 0 && N > 0)
		{
			res.m = M;
			res.n = N;

			res.value.clear();
			res.value.resize(res.m, std::vector<double>(res.n));
			for (int i = 0; i < res.m; i++)
			{
				for (int j = 0; j < res.n; j++)
				{
					res.value[i][j] = 0;
				}
			}
		}
		return res;
	}
	void print(const MatrixObject& obj)
	{
		if (obj.m > 0 && obj.n > 0)
		{
			std::cout << "m:" << obj.m << std::endl;
			std::cout << "n:" << obj.n << std::endl;
			for (int i = 0; i < obj.m; i++)
			{
				for (int j = 0; j < obj.n; j++)
				{
					std::cout << obj.value[i][j] << ",";
				}
				std::cout << std::endl;
			}
		}
	}


}