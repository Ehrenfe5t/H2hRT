#pragma once


#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include <Windows.h>

namespace FileOperateStd {


	void CopyFileByName(const std::string& sourceFileName, const std::string& goalFileName, bool state);

	void RemoveFile(const std::string& name);
	/// <summary>
	/// 检查文件是否存在
	/// </summary>
	/// <param name="path">文件路径</param>
	/// <returns>存在返回true</returns>
	bool ExistFile(const char* path);

	/// <summary>
	/// 将字符串写入txt类型的文件
	/// </summary>
	/// <param name="path"></param>
	/// <param name="jf"></param>
	void WriteStringToTxtFile(const char* path, const std::string& jf);




	/// <summary>
	/// 批量删除文件
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="name1"></param>
	/// <param name="name2"></param>
	void RemoveManyFiles(int start, int end, std::string name1, std::string name2);

	std::string ReadCsvFile_Trim(std::string& str);

	/// <summary>
	/// C20 按行加载了csv文件的字符串
	/// </summary>
	/// <param name="fileName"></param>
	/// <param name="allFields"></param>
	bool LoadCsvFile(const std::string& fileName, std::vector<std::vector<std::string>>& allFields);

	/// <summary>
	/// C20 按行加载了txt文件的字符串
	/// </summary>
	/// <param name="fileName"></param>
	/// <param name="allFields"></param>
	void LoadTxtFile(const std::string& fileName, std::vector<std::string>& allFields);

	/// <summary>
	/// 文件夹下的文件名，文件夹里面的文件夹不做处理
	/// </summary>
	/// <param name="path"></param>
	/// <param name="files"></param>
	/// <returns></returns>
	bool GetFilesPeer(const std::string& path, std::vector<std::string>& files);

	void GetFilesOnlyCurDirectory(const std::string& path, std::vector<std::string>& files);
	int GetFilesNumOnlyCurDirectory(const std::string& path);
	/// <summary>
	/// 获取文件夹下所有的文件
	/// </summary>
	/// <param name="path"></param>
	/// <param name="files"></param>
	void GetFiles(const std::string& path, std::vector<std::string>& files);

	/// <summary>
	/// char*转多字节字符
	/// </summary>
	/// <param name="szString"></param>
	/// <returns></returns>
	//LPWSTR ConvertCharToLPWSTR(const char* szString)
	//{
	//	int dwLen = (int)strlen(szString) + 1;
	//	int nwLen = MultiByteToWideChar(CP_ACP, 0, szString, dwLen, NULL, 0);//算出合适的长度
	//	LPWSTR lpszPath = new WCHAR[dwLen];
	//	MultiByteToWideChar(CP_ACP, 0, szString, dwLen, lpszPath, nwLen);
	//	return lpszPath;
	//}

	//LPCWSTR ConvertCharToLPWSTR(const char* szString);

	const char* ConvertCharToLPWSTR(const char* szString);

	/// <summary>
	/// 创建一个文件夹
	/// </summary>
	/// <param name="folderPath"></param>
	/// <returns></returns>
	int CreateMkdir(const std::string& folderPath);


	void OnlyMkdir(const std::string& folderPath);

	int CreateMkdirNoTip(const std::string& folderPath);

	/// <summary>
	/// 根据文件名创建一个新的文件，如果已经存在则不创建,最好不要是\\a\\b\\c.txt这种
	/// </summary>
	/// <param name="path">文件名</param>
	/// <returns>创建成功返回true</returns>
	bool CreateNewFile(const char* path);

	/// <summary>
	/// c++17 复制文件，把src复制到des
	/// </summary>
	/// <param name="src"></param>
	/// <param name="des"></param>
	/// <returns></returns>
	int CopyFileC17(const std::string& src, const std::string& des);

	/// <summary>
	/// 删除所有文件，输入为文件夹名称
	/// </summary>
	/// <param name="dir"></param>
	void RemoveAllFiles(const std::string& dir);


}

