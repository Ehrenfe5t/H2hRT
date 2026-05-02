

#include"QzQFileBase.h"
#include<Windows.h>
#include<direct.h>
#include<filesystem>
#include<io.h>
#include<iostream>
#include<assert.h>

namespace FileOperateStd {


	/// <summary>
	/// 
	/// </summary>
	/// <param name="sourceFileName"></param>
	/// <param name="goalFileName"></param>
	/// <param name="state">false 表示覆盖</param>
	void CopyFileByName(const std::string& sourceFileName, const std::string& goalFileName, bool state) {
		CopyFile(ConvertCharToLPWSTR(sourceFileName.c_str()), ConvertCharToLPWSTR(goalFileName.c_str()), state);
	}

	/// <summary>
	/// 检查文件是否存在
	/// </summary>
	/// <param name="path">文件路径</param>
	/// <returns>存在返回true</returns>
	bool ExistFile(const char* path) {
		std::filesystem::path filePath = path;
		return std::filesystem::exists(filePath);
		//if (FILE* file = fopen(path, "r")) {
		//	fclose(file);
		//	return true;
		//}
		//else {
		//	return false;
		//}
	}

	/// <summary>
	/// 将字符串写入txt类型的文件
	/// </summary>
	/// <param name="path"></param>
	/// <param name="jf"></param>
	void WriteStringToTxtFile(const char* path, const std::string& jf) {
		if (ExistFile(path)) {
			remove(path);
		}
		std::ofstream fileOpen; //定义ofstream 对象
		//fileOpen.open(path, std::ofstream::app);//追加
		fileOpen.open(path);
		if (!fileOpen)
		{
			std::cout << "没有打开文件！" << std::endl;
			fileOpen.close();
			return;
		}
		fileOpen << jf;
		fileOpen.close();
	}


	void RemoveFile(const std::string& name) {
		remove(name.data());
	}

	/// <summary>
	/// 批量删除文件
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="name1"></param>
	/// <param name="name2"></param>
	void RemoveManyFiles(int start, int end, std::string name1, std::string name2) {
		std::string context;
		for (int i = start; i <= end; i++) {
			context.append(name1);
			context.append(std::to_string(i));
			context.append(name2);
			remove(context.data());
			context.clear();
		}
	}

	std::string ReadCsvFile_Trim(std::string& str)
	{
		str.erase(0, str.find_first_not_of(" \t\r\n"));
		str.erase(str.find_last_not_of(" \t\r\n") + 1);
		return str;
	}
	/// <summary>
	/// C20 按行加载了csv文件的字符串
	/// </summary>
	/// <param name="fileName"></param>
	/// <param name="allFields"></param>
	bool LoadCsvFile(const std::string& fileName, std::vector<std::vector<std::string>>& allFields) {
		if (!ExistFile(fileName.c_str())) {
			std::cout << fileName << " 文件不存在\n";
			return false;
		}
		std::ifstream fin(fileName);
		std::string line;
		while (std::getline(fin, line)) {
			std::istringstream sin(line);
			std::vector<std::string> fields;
			std::string field;
			while (std::getline(sin, field, ',')) {
				fields.push_back(field);
			}

			if (fields.size() > 0) {
				allFields.push_back(fields);
			}
		}
		fin.close();
		return true;
	}

	/// <summary>
	/// C20 按行加载了txt文件的字符串
	/// </summary>
	/// <param name="fileName"></param>
	/// <param name="allFields"></param>
	void LoadTxtFile(const std::string& fileName, std::vector<std::string>& allFields) {
		std::ifstream infile;
		infile.open(fileName.data());   //将文件流对象与文件连接起来 
		assert(infile.is_open());   //若失败,则输出错误消息,并终止程序运行 
		std::string s;
		while (getline(infile, s))
			allFields.push_back(s.c_str());
		infile.close();             //关闭文件输入流
	}

	/// <summary>
	/// 文件夹下的文件名，文件夹里面的文件夹不做处理
	/// </summary>
	/// <param name="path"></param>
	/// <param name="files"></param>
	/// <returns></returns>
	bool GetFilesPeer(const std::string& path, std::vector<std::string>& files) {
		if (_access(path.c_str(), 0) == -1) {
			return false;
		}

		// 用来存储文件信息的结构体，在头文件 <io.h>
		struct _finddata_t fileinfo;  // _finddata_t 这是一个struct类，c++中可以不要前面的struct的
		intptr_t hFile = 0;
		std::string p;  // 不在这p(path)初始化
		// 第一次查找
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
			do {
				// 如果找到的是文件夹
				if ((fileinfo.attrib & _A_SUBDIR)) {
					continue;

					//// 不想进入文件夹，就在这里continue
					//if (std::strcmp(fileinfo.name, ".") != 0 && std::strcmp(fileinfo.name, "..") != 0) {
					//	// 进入查找
					//	files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					//	GetFiles(p.assign(path).append("\\").append(fileinfo.name), files);
					//}
				}
				// 如果找到的不是文件夹
				else {
					// 保存的是文件名
					files.push_back(p.assign(fileinfo.name));
					// 也可以是保存绝对路径
					// files.push_back(p.assign(path).append("\\").append(fileinfo.name));  
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			// 结束查找
			_findclose(hFile);
		}
		return true;
	}

	/// <summary>
	/// 获取文件夹下所有的文件
	/// </summary>
	/// <param name="path"></param>
	/// <param name="files"></param>
	void GetFiles(const std::string& path, std::vector<std::string>& files) {
		// 用来存储文件信息的结构体，在头文件 <io.h>
		struct _finddata_t fileinfo;  // _finddata_t 这是一个struct类，c++中可以不要前面的struct的
		intptr_t hFile = 0;
		std::string p;  // 不在这p(path)初始化
		// 第一次查找
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
			do {
				// 如果找到的是文件夹
				if ((fileinfo.attrib & _A_SUBDIR)) {
					// 不想进入文件夹，就在这里continue
					if (std::strcmp(fileinfo.name, ".") != 0 && std::strcmp(fileinfo.name, "..") != 0) {
						// 进入查找
						files.push_back(p.assign(path).append("\\").append(fileinfo.name));
						GetFiles(p.assign(path).append("\\").append(fileinfo.name), files);
					}
				}
				// 如果找到的不是文件夹
				else {
					// 保存的是文件名
					files.push_back(p.assign(fileinfo.name));
					// 也可以是保存绝对路径
					// files.push_back(p.assign(path).append("\\").append(fileinfo.name));  
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			// 结束查找
			_findclose(hFile);
		}
	}




	

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

	const char* ConvertCharToLPWSTR(const char* szString)
	{
		return szString;
	}


	//LPCWSTR ConvertCharToLPWSTR(const char* szString) {
	//	if (szString == nullptr) {
	//		return nullptr;
	//	}
	//	int size_needed = MultiByteToWideChar(CP_ACP, 0, szString, -1, nullptr, 0);
	//	std::wstring wstr(size_needed, 0);
	//	MultiByteToWideChar(CP_ACP, 0, szString, -1, &wstr[0], size_needed);
	//	return wstr.c_str();
	//}

	/// <summary>
	/// 创建一个文件夹
	/// </summary>
	/// <param name="folderPath"></param>
	/// <returns></returns>
	int CreateMkdir(const std::string& folderPath) {
		bool flag1 = RemoveDirectory(ConvertCharToLPWSTR(folderPath.c_str()));
		if (!flag1) {
			printf("删除原来的文件夹失败、\n");
		}
		//重新创建文件夹
		bool flag2 = CreateDirectory(ConvertCharToLPWSTR(folderPath.c_str()), NULL);
		if (flag2)
			return 1;
		printf("创建文件夹失败、\n");
		return 0;
	}

	void OnlyMkdir(const std::string& folderPath) {
		if (!std::filesystem::exists(folderPath)) {
			CreateDirectory(ConvertCharToLPWSTR(folderPath.c_str()), NULL);
		}
	}

	int CreateMkdirNoTip(const std::string& folderPath) {
		bool flag1 = RemoveDirectory(ConvertCharToLPWSTR(folderPath.c_str()));
		if (!flag1) {
			//printf("删除原来的文件夹失败、\n");
		}
		//重新创建文件夹
		bool flag2 = CreateDirectory(ConvertCharToLPWSTR(folderPath.c_str()), NULL);
		if (flag2)
			return 1;
		//printf("创建文件夹失败、\n");
		return 0;
	}

	/// <summary>
	/// 根据文件名创建一个新的文件，如果已经存在则不创建,最好不要是\\a\\b\\c.txt这种
	/// </summary>
	/// <param name="path">文件名</param>
	/// <returns>创建成功返回true</returns>
	bool CreateNewFile(const char* path) {
		if (ExistFile(path)) {
			return false;
		}
		std::ofstream ofs;
		ofs.open(path, std::ios::out);
		ofs.close();
		if (ExistFile(path)) {
			return true;
		}
		return false;
	}

	/// <summary>
	/// c++17 复制文件，把src复制到des
	/// </summary>
	/// <param name="src"></param>
	/// <param name="des"></param>
	/// <returns></returns>
	int CopyFileC17(const std::string& src, const std::string& des) {

		try
		{
			std::filesystem::copy(src, des, std::filesystem::copy_options::recursive);
		}
		catch (const std::exception&)
		{
			std::cout << "文件复制失败" << __FILE__ << "\t" << __LINE__ << std::endl;
			return 0;
		}
		return 1;
	}

	bool folder_exists(const std::string& path) {
		return std::filesystem::exists(path) && std::filesystem::is_directory(path);
	}

	/// <summary>
	/// 删除所有文件，输入为文件夹名称
	/// </summary>
	/// <param name="dir"></param>
	void RemoveAllFiles(const std::string& dir) {

		if (!folder_exists(dir)) {
			return;
		}

		//在目录后面加上"\\*.*"进行第一次搜索
		std::string newDir = dir + "\\*.*";
		//用于查找的句柄
		intptr_t handle;
		struct _finddata_t fileinfo;
		//第一次查找
		handle = _findfirst(newDir.c_str(), &fileinfo);

		if (handle == -1) {
			std::cout << "无文件" << std::endl;
			system("pause");
			return;
		}

		do
		{
			if (fileinfo.attrib & _A_SUBDIR) {//如果为文件夹，加上文件夹路径，再次遍历
				if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
					continue;

				// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
				newDir = dir + "\\" + fileinfo.name;
				RemoveAllFiles(newDir.c_str());//先遍历删除文件夹下的文件，再删除空的文件夹
				std::cout << newDir.c_str() << std::endl;
				if (_rmdir(newDir.c_str()) == 0) {//删除空文件夹
					std::cout << "delete empty dir success" << std::endl;
				}
				else {
					std::cout << "delete empty dir error" << std::endl;
				}
			}
			else {
				std::string file_path = dir + "\\" + fileinfo.name;
				std::cout << file_path.c_str() << std::endl;
				if (remove(file_path.c_str()) == 0) {//删除文件
					std::cout << "delete file success" << std::endl;
				}
				else {
					std::cout << "delete file error" << std::endl;
				}
			}
		} while (!_findnext(handle, &fileinfo));

		_findclose(handle);
		std::filesystem::remove_all(dir);
		return;
	}


	void GetFilesOnlyCurDirectory(const std::string& path, std::vector<std::string>& files) {
		// 用来存储文件信息的结构体，在头文件 <io.h>
		struct _finddata_t fileinfo;  // _finddata_t 这是一个struct类，c++中可以不要前面的struct的
		intptr_t hFile = 0;
		std::string p;  // 不在这p(path)初始化
		// 第一次查找
		if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
			do {
				// 如果找到的是文件夹
				if ((fileinfo.attrib & _A_SUBDIR)) {
					continue;
				}
				// 如果找到的不是文件夹
				else {
					// 保存的是文件名
					//files.push_back(p.assign(fileinfo.name));
					// 也可以是保存绝对路径
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			// 结束查找
			_findclose(hFile);
		}
	}

	int GetFilesNumOnlyCurDirectory(const std::string& path) {
		std::vector<std::string> files;
		GetFilesOnlyCurDirectory(path, files); 
		return (int)files.size();
	}

}