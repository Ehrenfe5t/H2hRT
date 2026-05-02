#pragma once


#include<string>

namespace FileOperateStd {

	void CreateMkdirS(const std::string& folderPath);

	/// <summary>
	/// 创建一个目录，如果是多级目录，则依次创建多级目录
	/// </summary>
	/// <param name="folderPath"></param>
	void CreateNewDirectorys(const std::string& folderPath);


}