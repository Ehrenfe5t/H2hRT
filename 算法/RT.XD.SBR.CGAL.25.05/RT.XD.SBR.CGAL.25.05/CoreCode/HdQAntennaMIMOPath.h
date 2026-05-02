#pragma once

#include"HdQAntennaSIMOPath.h"
#include<vector>
namespace AntennaMIMOPathStd {

	class AntennaMIMOPath
	{
	public:
		std::vector<AntennaSIMOPathStd::AntennaSIMOPath> paths;
		AntennaMIMOPath();
		~AntennaMIMOPath();

	private:

	};

	void FindCenterMultipathByDeduplicateRadius(AntennaMIMOPathStd::AntennaMIMOPath& antennaMIMOPath, double deduplicateRadius);

}