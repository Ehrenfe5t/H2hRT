
#include"QzQEquation2VariableObject.h"
#include"QzQEquation1VariableObject.h"
#include"QzQGlobalConstant.h"
#include<iostream>

namespace Equation2VariableObjectStd {


	Equation2VariableObject::Equation2VariableObject()
	{
		x1 = 0.0;
		x2 = 0.0;
	}

	Equation2VariableObject::~Equation2VariableObject()
	{
	}


	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByEqualizationMethod(
		int Number, double min_x1, double max_x1, double min_x2, double max_x2) {
		int Num = Number - 1;
		if (Num < 1) {
			Num = 1;
		}
		std::vector<double> x1 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, min_x1, max_x1);
		std::vector<double> x2 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, min_x2, max_x2);
		std::vector<Equation2VariableObject> res(x1.size());
		for (int i = 0;i<x1.size();++i) {
			res[i].x1 = x1[i];
			res[i].x2 = x2[i];
		}
		return res;
	}

	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, double min_x1, double max_x1, double min_x2, double max_x2) {
		int Num = Number - 1;
		if (Num < 1) {
			Num = 1;
		}
		std::vector<double> x1 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, min_x1, max_x1);
		std::vector<double> x2 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, min_x2, max_x2);
		std::vector<Equation2VariableObject> res(x1.size()*x2.size());
		for (int loop1 = 0; loop1 < x1.size(); ++loop1) {
			for (int loop2 = 0; loop2 < x2.size(); ++loop2) {
				int index = loop1 * (int)x2.size() + loop2;
				res[index].x1 = x1[loop1];
				res[index].x2 = x2[loop2];
			}
		}
		return res;
	}

	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::RangeDouble& x1RangeDouble,
		const RangeDoubleStd::RangeDouble& x2RangeDouble) {
		return GenerateSelfVariableObjectsByGridMethod(Number,
			x1RangeDouble.min_value, x1RangeDouble.max_value,
			x2RangeDouble.min_value, x2RangeDouble.max_value);
	}
	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range2Double& range) {
		return GenerateSelfVariableObjectsByGridMethod(Number, range.x1RangeDouble, range.x2RangeDouble);
	}

	void printf(const Equation2VariableObject& obj) {

		std::cout.precision(16);
		std::cout << "当前结果：" << std::endl;
		std::cout << "\tx1:" << obj.x1 << std::endl;
		std::cout << "\tx2:" << obj.x2 << std::endl;

	}


	int IndexOf(const Equation2VariableObject& obj, const std::vector<Equation2VariableObject>& objs) {

		for (int loop = 0; loop < objs.size(); ++loop) {
			if (abs(objs[loop].x1 - obj.x1) < GlobalConstantStd::Eps) {
				if (abs(objs[loop].x2 - obj.x2) < GlobalConstantStd::Eps) {
					return loop;
				}
			}

			//double d1 = objs[loop].x1 - obj.x1;
			//double d2 = objs[loop].x2 - obj.x2;
			//
			//double d3 = sqrt(d1 * d1 + d2 * d2);
			//if (d3 < 10 * GlobalConstantStd::Eps) {
			//	return loop;
			//}
		}

		return -1;
	}

	bool Add(const Equation2VariableObject& obj, std::vector<Equation2VariableObject>& objs) {
		int index = IndexOf(obj, objs);
		if (index == -1) {
			objs.emplace_back(obj);
			return true;
		}
		//已经有了不在添加
		return false;
	}


}