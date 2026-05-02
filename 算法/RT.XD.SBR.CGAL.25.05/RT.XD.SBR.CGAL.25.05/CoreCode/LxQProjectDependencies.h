#pragma once

#include<string>
#include<iostream>
#include<sstream>

namespace ProjectDependenciesStd {

	/// <summary>
	/// ¥Ú”°Ã· æ
	/// </summary>
	/// <param name="context"></param>
	/// <param name="locationInformationRequired"></param>
	/// <param name="file"></param>
	/// <param name="line"></param>
	/// <returns></returns>
	void DisplayPromptOrReason(const std::string& context, bool locationInformationRequired, const char* file, int line);

	void RunExe(const std::string& fileName, const std::string& path);

	bool RunExe2(const std::string& fileName, const std::string& dirName);
}
