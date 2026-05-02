


#include"QzQEquation1VariableObject.h"
#include"QzQGlobalConstant.h"
#include<iostream>

namespace Equation1VariableObjectStd {

	Equation1VariableObject::Equation1VariableObject()
	{
		x1 = 0.0;
	}

	Equation1VariableObject::~Equation1VariableObject()
	{
	}


	std::vector<double> GenerateSelfVariableObjectsByEqualizationMethod_1(int Num, double min_x1, double max_x1) {
		std::vector<double> x;
		{
			double dt = (max_x1 - min_x1) / Num;
			for (double loop_x1 = min_x1; loop_x1 <= max_x1; loop_x1 = loop_x1 + dt) {
				x.emplace_back(loop_x1);
			}
		}
		return x;
	}

	std::vector<double> GenerateSelfVariableObjectsByEqualizationMethod_1(int Num, const RangeDoubleStd::RangeDouble& range) {
		return GenerateSelfVariableObjectsByEqualizationMethod_1(Num, range.min_value, range.max_value);
	}

	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, double min_x1, double max_x1) {
		int Num = Number - 1;
		if (Num < 1) {
			Num = 1;
		}
		std::vector<double> x1 = GenerateSelfVariableObjectsByEqualizationMethod_1(Num, min_x1, max_x1);
		std::vector<Equation1VariableObject> res(x1.size());
		for (int loop1 = 0; loop1 < x1.size(); ++loop1) {
			res[loop1].x1 = x1[loop1];
		}
		return res;
	}
	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::RangeDouble& x1RangeDouble) {
		return  GenerateSelfVariableObjectsByGridMethod(Number, 
			x1RangeDouble.min_value, x1RangeDouble.max_value);
	}

	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range1Double& range) {
		return GenerateSelfVariableObjectsByGridMethod(Number,range.x1RangeDouble);
	}

	int IndexOf(const Equation1VariableObject& obj, const std::vector<Equation1VariableObject>& objs) {

		for (int loop = 0; loop<objs.size();++loop) {
			if (abs(objs[loop].x1 - obj.x1) < GlobalConstantStd::Eps) {
				return loop;
			}
		}

		return -1;
	}

	bool Add(const Equation1VariableObject& obj, std::vector<Equation1VariableObject>& objs) {
		int index = IndexOf(obj, objs);
		if (index == -1) {
			objs.emplace_back(obj);
			return true;
		}
		//已经有了不在添加
		return false;
	}

	void printf(const Equation1VariableObject& obj) {

		std::cout.precision(16);
		std::cout << "当前结果：" << std::endl;
		std::cout << "\tx1:" << obj.x1 << std::endl;

	}
}