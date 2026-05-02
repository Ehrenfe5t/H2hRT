
#include"QzQExampleOfSolving3DdimensionalEquation.h"
#include"LxQMatrixOperator.h"
#include"QzQEquation3VariableObject.h"
#include"QzQFunctionOperator.h"
#include<iostream>

namespace ExampleOfSolving3DdimensionalEquationStd {

	/// <summary>
	/// 方程式个数
	/// </summary>
	const int NumberOfEquations = 3;

	/// <summary>
	/// 变量个数
	/// </summary>
	const int NumberOfVariable = 3;

	/// <summary>
	/// 原方程组第1个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_0(const Equation3VariableObjectStd::Equation3VariableObject& input) {
		return input.x1 + 2.0 * input.x2 - input.x3 - 5;
	}
	/// <summary>
	/// 原方程组第2个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_1(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 2.0 * input.x1 - input.x2 + 3.0 * input.x3 - 2;
	}
	/// <summary>
	/// 原方程组第3个方程式
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	double CalFun_2(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 3.0 * input.x1 + input.x2 - 2.0 * input.x3 - 1;
	}

	/// <summary>
	/// 原方程组计算结果
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::vector<double> CalFun(const Equation3VariableObjectStd::Equation3VariableObject& input) {
		//几个方程式就有几个结果
		std::vector<double> res(NumberOfEquations);
		res[0] = CalFun_0(input);
		res[1] = CalFun_1(input);
		res[2] = CalFun_2(input);
		return res;
	}

	bool CheckFun(const Equation3VariableObjectStd::Equation3VariableObject& input) {
		std::vector<double> res = CalFun(input);
		return FunctionOperatorStd::CheckFunction(res);
	}

	double CalJacobianDeterminant_0_0(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 1;
	}
	double CalJacobianDeterminant_0_1(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 2;
	}
	double CalJacobianDeterminant_0_2(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return -1;
	}
	double CalJacobianDeterminant_1_0(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 2;
	}
	double CalJacobianDeterminant_1_1(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return -1;
	}
	double CalJacobianDeterminant_1_2(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 3;
	}
	double CalJacobianDeterminant_2_0(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 3;
	}
	double CalJacobianDeterminant_2_1(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return 1;
	}
	double CalJacobianDeterminant_2_2(const Equation3VariableObjectStd::Equation3VariableObject& input) {

		return -2;
	}

	MatrixObjectStd::MatrixObject CalJacobianDeterminant(
		const Equation3VariableObjectStd::Equation3VariableObject& input) {

		auto jacobianDeterminant = MatrixObjectStd::Init(NumberOfEquations, NumberOfVariable);
		jacobianDeterminant.value[0][0] = CalJacobianDeterminant_0_0(input);
		jacobianDeterminant.value[0][1] = CalJacobianDeterminant_0_1(input);
		jacobianDeterminant.value[0][2] = CalJacobianDeterminant_0_2(input);
		jacobianDeterminant.value[1][0] = CalJacobianDeterminant_1_0(input);
		jacobianDeterminant.value[1][1] = CalJacobianDeterminant_1_1(input);
		jacobianDeterminant.value[1][2] = CalJacobianDeterminant_1_2(input);
		jacobianDeterminant.value[2][0] = CalJacobianDeterminant_2_0(input);
		jacobianDeterminant.value[2][1] = CalJacobianDeterminant_2_1(input);
		jacobianDeterminant.value[2][2] = CalJacobianDeterminant_2_2(input);
		return jacobianDeterminant;
	}

	bool CalNext(const Equation3VariableObjectStd::Equation3VariableObject& input,
		Equation3VariableObjectStd::Equation3VariableObject& result) {

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
		result.x3 = input.x3 - temp1.value[2][0];
		return true;
	}

	bool GuessTrueOneResult(
		int curlevel,
		int maxLevel,
		const Equation3VariableObjectStd::Equation3VariableObject& input,
		Equation3VariableObjectStd::Equation3VariableObject& result) {
		if (curlevel > maxLevel) {
			return false;
		}
		//第1步，检查是否是满足方程组的，如果是则返回true，如果不是则开始迭代
		if (CheckFun(input)) {
			result = input;
			return true;
		}
		Equation3VariableObjectStd::Equation3VariableObject input_next;
		if (!CalNext(input, input_next)) {
			return false;
		}
		return GuessTrueOneResult(curlevel + 1, maxLevel, input_next, result);
	}

	void Demo01() {
		//猜测的值
		//单个变量的离散数量
		int numbersOfGuess = 3;
		//最大的迭代次数
		int maxLevel = 10;
		RangeDoubleStd::Range3Double range3Double;
		////变量1的取值区间
		//range3Double.x1RangeDouble.min_value = 0.0;
		//range3Double.x1RangeDouble.max_value = 4.0;
		////变量2的取值区间
		//range3Double.x2RangeDouble.min_value = 0.0;
		//range3Double.x2RangeDouble.max_value = 4.0;
		//
		//range3Double.x3RangeDouble.min_value = 0.0;
		//range3Double.x3RangeDouble.max_value = 4.0;
		auto variableObjectsOfGuess = Equation3VariableObjectStd::GenerateSelfVariableObjectsByGridMethod(
			numbersOfGuess, range3Double);
		for (int i = 0; i < variableObjectsOfGuess.size(); ++i) {
			Equation3VariableObjectStd::Equation3VariableObject result;
			if (GuessTrueOneResult(0, maxLevel, variableObjectsOfGuess[i], result)) {
				//std::cout << i << std::endl;
				Equation3VariableObjectStd::printf(result);
			}
			else {
				//std::cout << i <<"==========================================" << std::endl;
			}
		}


	}

}