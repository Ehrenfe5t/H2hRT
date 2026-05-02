
#include"LxQMatrixOperator.h"
#include<iostream>
#include<vector>

namespace MatrixOperatorStd {


	MatrixObjectStd::MatrixObject Trans(const MatrixObjectStd::MatrixObject& a)//矩阵转置
	{
		MatrixObjectStd::MatrixObject mat;
		if (a.m > 0 && a.n > 0)
		{
			mat = MatrixObjectStd::Init(a.n, a.m);
			
			for (int i = 0; i < mat.m; i++)
			{
				for (int j = 0; j < mat.n; j++)
				{
					mat.value[i][j] = a.value[j][i];
				}
			}
		}
		return mat;
	}
	MatrixObjectStd::MatrixObject Multiply(const MatrixObjectStd::MatrixObject& a, const MatrixObjectStd::MatrixObject& b)//两个矩阵相乘
	{
		MatrixObjectStd::MatrixObject c;
		if (a.n != b.m)
		{
			std::cout << "两个矩阵无法相乘！" << std::endl;
		}
		else
		{
			c = MatrixObjectStd::Init(a.m, b.n);
			for (int i = 0; i < c.m; i++)
			{
				for (int j = 0; j < c.n; j++)
				{
					c.value[i][j] = 0;
					for (int k = 0; k < a.n; k++)
						c.value[i][j] += a.value[i][k] * b.value[k][j];
				}
			}
		}
		return c;
	}
	int Rank(const MatrixObjectStd::MatrixObject& a0)//求矩阵的秩
	{
		int r = 0, m1 = 0;
		MatrixObjectStd::MatrixObject a;
		a = MatrixObjectStd::Init(a0.m, a0.n);

		for (int i = 0; i < a.m; i++)
			for (int j = 0; j < a.n; j++)
				a.value[i][j] = a0.value[i][j];
		if (a.m > a.n)
			m1 = a.n;
		else
			m1 = a.m;
		for (int i = 0; i < m1; i++)//从上到下，将主对角线元素化为1，左下角矩阵化为0
		{
			double aii = a.value[i][i];//记录每行主对角线元素
			if (abs(aii) <= (1e-6))//如果为0
			{
				int k;
				for (k = i + 1; k < a.m; k++)
				{
					double aki = a.value[k][i];//记录每行对应第i行主对角线元素的元素
					if (abs(aki) > (1e-6))//如果不为0
					{
						for (int j = 0; j < a.n; j++)
						{
							a.value[i][j] -= a.value[k][j];
						}
						double ai = a.value[i][i];
						for (int j = 0; j < a.n; j++)
							a.value[i][j] /= ai;
						break;
					}
				}
			}
			else  //主元素不为0
			{
				for (int j = 0; j < a.n; j++)
					a.value[i][j] /= aii;
			}
			for (int l = i + 1; l < a.m; l++)
			{
				double ali = a.value[l][i];
				if (abs(ali) > (1e-6))
				{
					for (int j = 0; j < a.n; j++)
					{
						a.value[l][j] -= ali * a.value[i][j];
					}
				}
			}
		}
		for (int i = m1 - 1; i >= 0; i--)//从下到上，将右上角矩阵化为0
		{
			for (int k = i - 1; k >= 0; k--)
			{
				double aki = a.value[k][i];
				if (abs(aki) > (1e-6))
				{
					for (int j = a.n - 1; j >= 0; j--)
					{
						a.value[k][j] -= aki * a.value[i][j];
					}
				}
			}
		}

		for (int i = 0; i < a.m; i++)
		{
			for (int j = 0; j < a.n; j++)
			{
				if (a.value[i][j] > (1e-6))
				{
					r++; break;
				}
			}
		}
		return r;
	}
	MatrixObjectStd::MatrixObject inv(const MatrixObjectStd::MatrixObject& a0)//方阵求逆
	{
		MatrixObjectStd::MatrixObject c;
		if (a0.m != a0.n)
		{
			std::cout << "非方阵，不可逆！" << std::endl;
		}
		else
		{
			if (Rank(a0) != a0.m)
				std::cout << "该方阵为奇异矩阵，不可逆！" << std::endl;
			else
			{
				MatrixObjectStd::MatrixObject a;
				a = MatrixObjectStd::Init(a0.m, a0.n);

				for (int i = 0; i < a.m; i++)
					for (int j = 0; j < a.n; j++)
						a.value[i][j] = a0.value[i][j];

				c = MatrixObjectStd::Init(a.m, a.n);
				for (int i = 0; i < c.m; i++)
				{
					for (int j = 0; j < c.n; j++)
					{
						if (i == j) c.value[i][j] = 1;
						else c.value[i][j] = 0;
					}
				}
				for (int i = 0; i < a.m; i++)
				{
					double aii = a.value[i][i];//记录每行主对角线元素
					if (abs(aii) <= (1e-6))//如果为0
					{
						int k;
						for (k = i + 1; k < a.m; k++)
						{
							double aki = a.value[k][i];//记录每行主对角线元素
							if (abs(aki) > (1e-6))//如果不为0
							{
								for (int j = 0; j < a.n; j++)
								{
									a.value[i][j] -= a.value[k][j];
									c.value[i][j] -= c.value[k][j];
								}
								aii = a.value[i][i];
								for (int j = 0; j < a.n; j++)
								{
									a.value[i][j] /= aii;
									c.value[i][j] /= aii;
								}
								break;
							}
						}
					}
					else//如果不为0
					{
						for (int j = 0; j < a.n; j++)
						{
							a.value[i][j] /= aii;
							c.value[i][j] /= aii;
						}
					}
					for (int l = i + 1; l < a.m; l++)
					{
						double ali = a.value[l][i];
						if (abs(ali) > (1e-6))
						{
							for (int j = 0; j < a.n; j++)
							{
								a.value[l][j] -= ali * a.value[i][j];
								c.value[l][j] -= ali * c.value[i][j];
							}
						}
					}
				}
				for (int i = a.m - 1; i >= 0; i--)
				{
					for (int k = i - 1; k >= 0; k--)
					{
						double aki = a.value[k][i];
						if (abs(aki) > (1e-6))
						{
							for (int j = a.n - 1; j >= 0; j--)
							{
								a.value[k][j] -= aki * a.value[i][j];
								c.value[k][j] -= aki * c.value[i][j];
							}
						}
					}
				}
			}
		}
		return c;
	}

	bool inv_safe(const MatrixObjectStd::MatrixObject& a0, MatrixObjectStd::MatrixObject& c) {

		if (a0.m != a0.n)
		{
			std::cout << "非方阵，不可逆！" << std::endl;
			return false;
		}
		else
		{
			if (Rank(a0) != a0.m) {
				//std::cout << "该方阵为奇异矩阵，不可逆！" << std::endl;
				return false;
			}
			else
			{
				MatrixObjectStd::MatrixObject a;
				a = MatrixObjectStd::Init(a0.m, a0.n);

				for (int i = 0; i < a.m; i++)
					for (int j = 0; j < a.n; j++)
						a.value[i][j] = a0.value[i][j];

				c = MatrixObjectStd::Init(a.m, a.n);
				for (int i = 0; i < c.m; i++)
				{
					for (int j = 0; j < c.n; j++)
					{
						if (i == j) c.value[i][j] = 1;
						else c.value[i][j] = 0;
					}
				}
				for (int i = 0; i < a.m; i++)
				{
					double aii = a.value[i][i];//记录每行主对角线元素
					if (abs(aii) <= (1e-6))//如果为0
					{
						int k;
						for (k = i + 1; k < a.m; k++)
						{
							double aki = a.value[k][i];//记录每行主对角线元素
							if (abs(aki) > (1e-6))//如果不为0
							{
								for (int j = 0; j < a.n; j++)
								{
									a.value[i][j] -= a.value[k][j];
									c.value[i][j] -= c.value[k][j];
								}
								aii = a.value[i][i];
								for (int j = 0; j < a.n; j++)
								{
									a.value[i][j] /= aii;
									c.value[i][j] /= aii;
								}
								break;
							}
						}
					}
					else//如果不为0
					{
						for (int j = 0; j < a.n; j++)
						{
							a.value[i][j] /= aii;
							c.value[i][j] /= aii;
						}
					}
					for (int l = i + 1; l < a.m; l++)
					{
						double ali = a.value[l][i];
						if (abs(ali) > (1e-6))
						{
							for (int j = 0; j < a.n; j++)
							{
								a.value[l][j] -= ali * a.value[i][j];
								c.value[l][j] -= ali * c.value[i][j];
							}
						}
					}
				}
				for (int i = a.m - 1; i >= 0; i--)
				{
					for (int k = i - 1; k >= 0; k--)
					{
						double aki = a.value[k][i];
						if (abs(aki) > (1e-6))
						{
							for (int j = a.n - 1; j >= 0; j--)
							{
								a.value[k][j] -= aki * a.value[i][j];
								c.value[k][j] -= aki * c.value[i][j];
							}
						}
					}
				}
			}
		}
		return true;
	}

	MatrixObjectStd::MatrixObject pinv(const MatrixObjectStd::MatrixObject& a0)//矩阵求广义逆
	{
		MatrixObjectStd::MatrixObject A;
		A = MatrixObjectStd::Init(a0.m, a0.n);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < A.n; j++)
				A.value[i][j] = a0.value[i][j];
		int M = 0;
		if (A.m > A.n)
			M = A.n;
		else
			M = A.m;
		MatrixObjectStd::MatrixObject E;
		E = MatrixObjectStd::InitEye(A.m);

		for (int i = 0; i < M; i++)
		{
			double aii = A.value[i][i];//记录每行主对角线元素
			if (abs(aii) <= (1e-6))//如果为0
			{
				int k;
				for (k = i + 1; k < A.m; k++)
				{
					double aki = A.value[k][i];//记录每行对应第i行主对角线元素的元素
					if (abs(aki) > (1e-6))//如果不为0
					{
						for (int j = 0; j < A.n; j++)
						{
							A.value[i][j] -= A.value[k][j];
						}
						for (int j = 0; j < A.m; j++)
						{
							E.value[i][j] -= E.value[k][j];
						}
						double ai = A.value[i][i];
						for (int j = 0; j < A.n; j++)
						{
							A.value[i][j] /= ai;
						}
						for (int j = 0; j < A.m; j++)
						{
							E.value[i][j] /= ai;
						}
						break;
					}
				}
			}
			else  //主元素不为0
			{
				for (int j = 0; j < A.n; j++)
				{
					A.value[i][j] /= aii;
				}
				for (int j = 0; j < A.m; j++)
				{
					E.value[i][j] /= aii;
				}
			}
			for (int l = i + 1; l < A.m; l++)
			{
				double ali = A.value[l][i];
				if (abs(ali) > (1e-6))
				{
					for (int j = 0; j < A.n; j++)
					{
						A.value[l][j] -= ali * A.value[i][j];
					}
					for (int j = 0; j < A.m; j++)
					{
						E.value[l][j] -= ali * E.value[i][j];
					}
				}
				else
					continue;//为零，则下一行
			}
		}
		for (int i = M - 1; i >= 0; i--)
		{
			for (int k = i - 1; k >= 0; k--)
			{
				double aki = A.value[k][i];
				if (abs(aki) > (1e-6))
				{
					for (int j = A.n - 1; j >= 0; j--)
					{
						A.value[k][j] -= aki * A.value[i][j];
					}
					for (int j = A.m - 1; j >= 0; j--)
					{
						E.value[k][j] -= aki * E.value[i][j];
					}
				}
			}
		}

		int rA = Rank(A);
		for (int q = 0; q < rA; q++)
		{
			if (abs(A.value[q][q]) <= (1e-6))//主对角线元素若为0，需要交换行或交换列
			{
				for (int i = q + 1; i < M; i++)
				{
					int biaoji = -1;
					if (A.value[i][i] == 1)
					{
						biaoji = i;
					}
					else if (abs(A.value[i][i]) <= (1e-6))//主对角线元素若为0
					{
						for (int j = 0; j < A.n; j++)
						{
							if (A.value[i][j] == 1)
							{
								biaoji = i;
								break;
							}
						}
					}
					if (biaoji > 0)//该行有1，已经找到其行号和列号
					{
						//将i行和q行交换
						for (int w = 0; w < A.n; w++)
						{
							double temp = A.value[q][w];
							A.value[q][w] = A.value[i][w];
							A.value[i][w] = temp;
						}
						for (int w = 0; w < A.m; w++)
						{
							double temp1 = E.value[q][w];
							E.value[q][w] = E.value[i][w];
							E.value[i][w] = temp1;
						}
					}
				}
			}
		}
		int z = rA, rB = 0, rC = 0;
		MatrixObjectStd::MatrixObject G, P, P1, B, C;
		G = MatrixObjectStd::Init(A.m, A.n);
		P = MatrixObjectStd::Init(E.m, E.n);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < A.n; j++)
			{
				G.value[i][j] = A.value[i][j];
			}
		for (int i = 0; i < E.m; i++)
			for (int j = 0; j < E.n; j++)
			{
				P.value[i][j] = E.value[i][j];
			}
		P1 = inv(P);
		B = MatrixObjectStd::Init(A.m, z);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < z; j++)
				B.value[i][j] = P1.value[i][j];
		rB = Rank(B);

		C = MatrixObjectStd::Init(z, A.n);
		for (int i = 0; i < z; i++)
			for (int j = 0; j < A.n; j++)
				C.value[i][j] = G.value[i][j];
		rC = Rank(C);
		MatrixObjectStd::MatrixObject Bt = Trans(B), Ct = Trans(C);
		MatrixObjectStd::MatrixObject A1 = Multiply(Multiply(Ct, inv(Multiply(C, Ct))), Multiply(inv(Multiply(Bt, B)), Bt));
		return A1;
	}

	bool pinv_safe(const MatrixObjectStd::MatrixObject& a0, MatrixObjectStd::MatrixObject&result)
	{
		MatrixObjectStd::MatrixObject A;
		A = MatrixObjectStd::Init(a0.m, a0.n);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < A.n; j++)
				A.value[i][j] = a0.value[i][j];
		int M = 0;
		if (A.m > A.n)
			M = A.n;
		else
			M = A.m;
		MatrixObjectStd::MatrixObject E;
		E = MatrixObjectStd::InitEye(A.m);

		for (int i = 0; i < M; i++)
		{
			double aii = A.value[i][i];//记录每行主对角线元素
			if (abs(aii) <= (1e-6))//如果为0
			{
				int k;
				for (k = i + 1; k < A.m; k++)
				{
					double aki = A.value[k][i];//记录每行对应第i行主对角线元素的元素
					if (abs(aki) > (1e-6))//如果不为0
					{
						for (int j = 0; j < A.n; j++)
						{
							A.value[i][j] -= A.value[k][j];
						}
						for (int j = 0; j < A.m; j++)
						{
							E.value[i][j] -= E.value[k][j];
						}
						double ai = A.value[i][i];
						for (int j = 0; j < A.n; j++)
						{
							A.value[i][j] /= ai;
						}
						for (int j = 0; j < A.m; j++)
						{
							E.value[i][j] /= ai;
						}
						break;
					}
				}
			}
			else  //主元素不为0
			{
				for (int j = 0; j < A.n; j++)
				{
					A.value[i][j] /= aii;
				}
				for (int j = 0; j < A.m; j++)
				{
					E.value[i][j] /= aii;
				}
			}
			for (int l = i + 1; l < A.m; l++)
			{
				double ali = A.value[l][i];
				if (abs(ali) > (1e-6))
				{
					for (int j = 0; j < A.n; j++)
					{
						A.value[l][j] -= ali * A.value[i][j];
					}
					for (int j = 0; j < A.m; j++)
					{
						E.value[l][j] -= ali * E.value[i][j];
					}
				}
				else
					continue;//为零，则下一行
			}
		}
		for (int i = M - 1; i >= 0; i--)
		{
			for (int k = i - 1; k >= 0; k--)
			{
				double aki = A.value[k][i];
				if (abs(aki) > (1e-6))
				{
					for (int j = A.n - 1; j >= 0; j--)
					{
						A.value[k][j] -= aki * A.value[i][j];
					}
					for (int j = A.m - 1; j >= 0; j--)
					{
						E.value[k][j] -= aki * E.value[i][j];
					}
				}
			}
		}

		int rA = Rank(A);
		for (int q = 0; q < rA; q++)
		{
			if (abs(A.value[q][q]) <= (1e-6))//主对角线元素若为0，需要交换行或交换列
			{
				for (int i = q + 1; i < M; i++)
				{
					int biaoji = -1;
					if (A.value[i][i] == 1)
					{
						biaoji = i;
					}
					else if (abs(A.value[i][i]) <= (1e-6))//主对角线元素若为0
					{
						for (int j = 0; j < A.n; j++)
						{
							if (A.value[i][j] == 1)
							{
								biaoji = i;
								break;
							}
						}
					}
					if (biaoji > 0)//该行有1，已经找到其行号和列号
					{
						//将i行和q行交换
						for (int w = 0; w < A.n; w++)
						{
							double temp = A.value[q][w];
							A.value[q][w] = A.value[i][w];
							A.value[i][w] = temp;
						}
						for (int w = 0; w < A.m; w++)
						{
							double temp1 = E.value[q][w];
							E.value[q][w] = E.value[i][w];
							E.value[i][w] = temp1;
						}
					}
				}
			}
		}

		if (rA == 0) {
			return false;
		}

		int z = rA, rB = 0, rC = 0;
		MatrixObjectStd::MatrixObject G, P, P1, B, C;
		G = MatrixObjectStd::Init(A.m, A.n);
		P = MatrixObjectStd::Init(E.m, E.n);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < A.n; j++)
			{
				G.value[i][j] = A.value[i][j];
			}
		for (int i = 0; i < E.m; i++)
			for (int j = 0; j < E.n; j++)
			{
				P.value[i][j] = E.value[i][j];
			}
		if (!inv_safe(P,P1)) {
			return false;
		}
		//P1 = inv(P);
		B = MatrixObjectStd::Init(A.m, z);

		for (int i = 0; i < A.m; i++)
			for (int j = 0; j < z; j++)
				B.value[i][j] = P1.value[i][j];
		rB = Rank(B);

		C = MatrixObjectStd::Init(z, A.n);
		for (int i = 0; i < z; i++)
			for (int j = 0; j < A.n; j++)
				C.value[i][j] = G.value[i][j];
		rC = Rank(C);
		MatrixObjectStd::MatrixObject Bt = Trans(B), Ct = Trans(C), temp_1, temp_2;
		if (!inv_safe(Multiply(C, Ct), temp_1)) {
			return false;
		}
		if (!inv_safe(Multiply(Bt, B), temp_2)) {
			return false;
		}

		result = Multiply(Multiply(Ct, temp_1), Multiply(temp_2, Bt));
		return true;
	}
}