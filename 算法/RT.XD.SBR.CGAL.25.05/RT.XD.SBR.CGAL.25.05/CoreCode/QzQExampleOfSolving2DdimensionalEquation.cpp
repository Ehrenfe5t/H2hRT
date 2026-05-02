
#include"QzQExampleOfSolving2DdimensionalEquation.h"
#include"LxQMatrixOperator.h"
#include"QzQEquation2VariableObject.h"
#include"QzQFunctionOperator.h"
#include<iostream>

namespace ExampleOfSolving2DdimensionalEquationStd {

	/// <summary>
	/// 方程式个数
	/// </summary>
	const int NumberOfEquations = 2;

	/// <summary>
	/// 变量个数
	/// </summary>
	const int NumberOfVariable = 2;

	/// <summary>
	/// 原方程组第1个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_0(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return 3 * input.x1 * input.x1 - 20 - input.x2;
	}
	/// <summary>
	/// 原方程组第2个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_1(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return 0.1 * input.x1 * input.x1 * input.x1 + 1 - input.x2;
	}
	/// <summary>
	/// 原方程组计算结果
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::vector<double> CalFun(const Equation2VariableObjectStd::Equation2VariableObject& input) {
		//几个方程式就有几个结果
		std::vector<double> res(NumberOfEquations);
		res[0] = CalFun_0(input);
		res[1] = CalFun_1(input);
		return res;
	}

	bool CheckFun(const Equation2VariableObjectStd::Equation2VariableObject& input) {
		std::vector<double> res = CalFun(input);
		return FunctionOperatorStd::CheckFunction(res);
	}

	double CalJacobianDeterminant_0_0(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return 6 * input.x1;
	}
	double CalJacobianDeterminant_0_1(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return -1;
	}
	double CalJacobianDeterminant_1_0(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return 0.3 * input.x1 * input.x1;
	}
	double CalJacobianDeterminant_1_1(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		return -1;
	}

	MatrixObjectStd::MatrixObject CalJacobianDeterminant(const Equation2VariableObjectStd::Equation2VariableObject& input) {

		auto jacobianDeterminant = MatrixObjectStd::Init(NumberOfEquations, NumberOfVariable);
		jacobianDeterminant.value[0][0] = CalJacobianDeterminant_0_0(input);
		jacobianDeterminant.value[0][1] = CalJacobianDeterminant_0_1(input);
		jacobianDeterminant.value[1][0] = CalJacobianDeterminant_1_0(input);
		jacobianDeterminant.value[1][1] = CalJacobianDeterminant_1_1(input);
		return jacobianDeterminant;
	}

	bool CalNext(const Equation2VariableObjectStd::Equation2VariableObject& input,
		Equation2VariableObjectStd::Equation2VariableObject& result) {

		auto jacobianDeterminant = CalJacobianDeterminant(input);
		MatrixObjectStd::MatrixObject jacobianDeterminant_inv;
		if (!MatrixOperatorStd::pinv_safe(jacobianDeterminant, jacobianDeterminant_inv)) {
			return false;
		}
		std::vector<double> fun_res = CalFun(input);
		auto fun_matrix = MatrixObjectStd::Init(fun_res, NumberOfEquations, 1);
		auto temp1 = MatrixOperatorStd::Multiply(jacobianDeterminant_inv, fun_matrix);
		result.x1 = input.x1 - temp1.value[0][0];
		result.x2 = input.x2 - temp1.value[1][0];
		return true;
	}

	bool GuessTrueOneResult(
		int curlevel,
		int maxLevel,
		const Equation2VariableObjectStd::Equation2VariableObject& input, 
		Equation2VariableObjectStd::Equation2VariableObject& result) {
		if (curlevel > maxLevel) {
			return false;
		}
		//第1步，检查是否是满足方程组的，如果是则返回true，如果不是则开始迭代
		if (CheckFun(input)) {
			result = input;
			return true;
		}
		Equation2VariableObjectStd::Equation2VariableObject input_next;
		if (!CalNext(input, input_next)) {
			return false;
		}
		return GuessTrueOneResult(curlevel + 1, maxLevel, input_next, result);
	}

	void Demo01() {
		//猜测的值
		//单个变量的离散数量
		int numbersOfGuess = 5;
		//最大的迭代次数
		int maxLevel = 10;
		//变量1的取值区间
		double min_x1_value = -10;
		double max_x1_value = 30;
		//变量2的取值区间
		double min_x2_value = -100;
		double max_x2_value = 3200;
		auto variableObjectsOfGuess = Equation2VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, min_x1_value, max_x1_value, min_x2_value, max_x2_value);
		for (int i = 0;i< variableObjectsOfGuess.size();++i) {
			Equation2VariableObjectStd::Equation2VariableObject result;
			if (GuessTrueOneResult(0, maxLevel,variableObjectsOfGuess[i], result)) {
				//std::cout << i << std::endl;
				Equation2VariableObjectStd::printf(result);
			}
			else {
				//std::cout << i <<"==========================================" << std::endl;
			}
		}
		

	}

}