#pragma once
#include<vector>
#include"DxQRangeDouble.h"

namespace Equation1VariableObjectStd {

	class Equation1VariableObject
	{
	public:
		double x1;
		Equation1VariableObject();
		~Equation1VariableObject();

	private:

	};

	std::vector<double> GenerateSelfVariableObjectsByEqualizationMethod_1(int Num, double min_x1, double max_x1);
	std::vector<double> GenerateSelfVariableObjectsByEqualizationMethod_1(int Num, const RangeDoubleStd::RangeDouble& range);
	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, double min_x1, double max_x1);
	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::RangeDouble& rangeDouble);
	std::vector<Equation1VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range1Double& range);

	int IndexOf(const Equation1VariableObject& obj, const std::vector<Equation1VariableObject>& objs);
	bool Add(const Equation1VariableObject& obj, std::vector<Equation1VariableObject>& objs);

	void printf(const Equation1VariableObject& obj);
}