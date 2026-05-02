
#include"QzQEquation3VariableObject.h"
#include"QzQEquation1VariableObject.h"
#include"QzQGlobalConstant.h"
#include<iostream>

namespace Equation3VariableObjectStd {


	Equation3VariableObject::Equation3VariableObject()
	{
		x1 = 0.0;
		x2 = 0.0;
		x3 = 0.0;
	}

	Equation3VariableObject::~Equation3VariableObject()
	{
	}

	std::vector<Equation3VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range3Double& range) {
		int Num = Number - 1;
		if (Num < 1) {
			Num = 1;
		}
		std::vector<double> x1 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, range.x1RangeDouble);
		std::vector<double> x2 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, range.x2RangeDouble);
		std::vector<double> x3 = Equation1VariableObjectStd::GenerateSelfVariableObjectsByEqualizationMethod_1(Num, range.x3RangeDouble);
		std::vector<Equation3VariableObject> res;
		for (int loop1 = 0; loop1 < x1.size(); ++loop1) {
			for (int loop2 = 0; loop2 < x2.size(); ++loop2) {
				for (int loop3 = 0; loop3 < x3.size(); ++loop3) {
					Equation3VariableObject obj;
					obj.x1 = x1[loop1];
					obj.x2 = x2[loop2];
					obj.x3 = x3[loop3];
					res.emplace_back(obj);
				}
			}
		}
		return res;
	}


	int IndexOf(const Equation3VariableObject& obj, const std::vector<Equation3VariableObject>& objs) {

		for (int loop = 0; loop < objs.size(); ++loop) {
			if (abs(objs[loop].x1 - obj.x1) < GlobalConstantStd::Eps) {
				if (abs(objs[loop].x2 - obj.x2) < GlobalConstantStd::Eps) {
					if (abs(objs[loop].x3 - obj.x3) < GlobalConstantStd::Eps) {
						return loop;
					}
				}
			}
		}

		return -1;
	}

	bool Add(const Equation3VariableObject& obj, std::vector<Equation3VariableObject>& objs) {
		int index = IndexOf(obj, objs);
		if (index == -1) {
			objs.emplace_back(obj);
			return true;
		}
		//已经有了不在添加
		return false;
	}

	void printf(const Equation3VariableObject& obj) {
		std::cout.precision(16);
		std::cout << "当前结果：" << std::endl;
		std::cout << "\tx1:" << obj.x1 << std::endl;
		std::cout << "\tx2:" << obj.x2 << std::endl;
		std::cout << "\tx3:" << obj.x3 << std::endl;

	}

}