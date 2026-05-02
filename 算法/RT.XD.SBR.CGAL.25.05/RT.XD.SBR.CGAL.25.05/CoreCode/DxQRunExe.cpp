

#include"DxQRunExe.h"

#include <iostream>
#include <filesystem>
#include<Windows.h>

namespace RunExeStd {

	void RunExe(const std::string& fileName, const std::string& path) {
		std::string curFileName = path + "\\" + fileName;
		SHELLEXECUTEINFO shExecInfo = { 0 };
		shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExecInfo.hwnd = NULL;
		shExecInfo.lpVerb = NULL;
		shExecInfo.lpFile = curFileName.c_str();
		shExecInfo.lpParameters = "";
		shExecInfo.lpDirectory = path.c_str();
		shExecInfo.nShow = SW_HIDE;
		shExecInfo.hInstApp = NULL;

		ShellExecuteEx(&shExecInfo);
		if (shExecInfo.hProcess) {
			WaitForSingleObject(shExecInfo.hProcess, INFINITE);
		}
	}

	bool RunExe2(const std::string& fileName, const std::string& dirName) {

		std::error_code ec;
		std::filesystem::path current_path = std::filesystem::current_path();

		// 设置运行目录
		std::filesystem::path run_dir(dirName);
		if (std::filesystem::exists(run_dir) && std::filesystem::is_directory(run_dir)) {
			std::filesystem::current_path(run_dir, ec);
			if (ec) {
				std::cerr << "Error: " << ec.message() << std::endl;
				std::filesystem::current_path(current_path, ec);
				return false;
			}
		}
		else {
			std::cerr << "Error: Directory does not exist." << std::endl;
			std::filesystem::current_path(current_path, ec);
			return false;
		}

		int result = std::system(fileName.c_str());
		if (result < 0) {
			std::cerr << "Execution failed." << std::endl;
			std::filesystem::current_path(current_path, ec);
			return false;
		}
		std::filesystem::current_path(current_path, ec);
		return true;
	}

}
