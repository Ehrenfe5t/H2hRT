#pragma once


#include<vector>
#include"DxQRangeDouble.h"

namespace Equation3VariableObjectStd {

	class Equation3VariableObject
	{
	public:
		double x1;
		double x2;
		double x3;
		Equation3VariableObject();
		~Equation3VariableObject();

	private:

	};

	std::vector<Equation3VariableObject> GenerateSelfVariableObjectsByGridMethod(
		int Number, const RangeDoubleStd::Range3Double& range);
	void printf(const Equation3VariableObject& obj);


	int IndexOf(const Equation3VariableObject& obj, const std::vector<Equation3VariableObject>& objs);
	bool Add(const Equation3VariableObject& obj, std::vector<Equation3VariableObject>& objs);
}