
#include"QzQDirectoryOperate.h"
#include"QzQFileBase.h"

#include"QzQDoWithString.h"

#include<filesystem>
#include<Windows.h>

namespace FileOperateStd {

	void CreateMkdirS(const std::string& folderPath) {
		std::vector<std::string>folders = DoWithStringStd::SplitWithStl(folderPath, "\\");
		if (folders.size() < 1) {
			return;
		}
		auto curFolderPath = folders[0];

		if (!std::filesystem::exists(curFolderPath)) {
			CreateDirectory(FileOperateStd::ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
		}
		for (int loop = 1; loop < folders.size(); loop++) {
			curFolderPath.append("\\");
			curFolderPath.append(folders[loop]);
			if (!std::filesystem::exists(curFolderPath)) {
				CreateDirectory(FileOperateStd::ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
			}
		}

	}

	/// <summary>
	/// 创建一个目录，如果是多级目录，则依次创建多级目录
	/// </summary>
	/// <param name="folderPath"></param>
	void CreateNewDirectorys(const std::string& folderPath) {
		std::vector<std::string>folders = DoWithStringStd::SplitWithStl(folderPath, "\\");
		if (folders.size() < 1) {
			return;
		}
		auto curFolderPath = folders[0];

		if (!std::filesystem::exists(curFolderPath)) {
			CreateDirectory(FileOperateStd::ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
		}
		for (int loop = 1; loop < folders.size(); loop++) {
			curFolderPath.append("\\");
			curFolderPath.append(folders[loop]);
			if (!std::filesystem::exists(curFolderPath)) {
				CreateDirectory(FileOperateStd::ConvertCharToLPWSTR(curFolderPath.c_str()), NULL);
			}
		}
	}

}

