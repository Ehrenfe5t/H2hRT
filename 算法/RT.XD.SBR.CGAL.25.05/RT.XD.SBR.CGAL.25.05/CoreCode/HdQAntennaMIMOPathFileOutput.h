#pragma once

#include"HdQAntennaMIMOPath.h"
#include<string>

namespace AntennaMIMOPathFileOutputStd {

	void AntennaMIMOPathFileOutput(
        bool switchOfPathInfo,
		const std::string& outPutDirectoryPathName,
		const AntennaMIMOPathStd::AntennaMIMOPath&antennaMIMOPath);

}