
#include"QzQFunctionOperator.h"
#include"QzQGlobalConstant.h"


namespace FunctionOperatorStd {


	bool CheckFunction(const std::vector<double>& results) {
		for (int i = 0; i < results.size(); ++i) {
			if (abs(results[i]) > GlobalConstantStd::Eps) {
				return false;
			}
		}
		return true;
	}

}