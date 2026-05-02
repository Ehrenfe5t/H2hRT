#pragma once

#include<vector>
#include"DxQRangeDouble.h"

namespace Equation2VariableObjectStd {

	class Equation2VariableObject
	{
	public:
		double x1;
		double x2;

		Equation2VariableObject();
		~Equation2VariableObject();

	private:

	};


	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByEqualizationMethod(
		int Number, double min_x1, double max_x1, double min_x2, double max_x2);
	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, double min_x1, double max_x1, double min_x2, double max_x2);
	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::RangeDouble& x1RangeDouble, 
		const RangeDoubleStd::RangeDouble& x2RangeDouble);
	std::vector<Equation2VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range2Double& range);
	void printf(const Equation2VariableObject& obj);


	int IndexOf(const Equation2VariableObject& obj, const std::vector<Equation2VariableObject>& objs);
	bool Add(const Equation2VariableObject& obj, std::vector<Equation2VariableObject>& objs);
}