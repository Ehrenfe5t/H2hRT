

#include"LxQProjectDependencies.h"

#include"DxQRunExe.h"
#include<iostream>

/// <summary>
/// 是否显示提示或者原因
/// </summary>
/// <returns></returns>
#define WhetherToDisplayPromptOrReason true
/// <summary>
/// 是否显示对应的源代码位置
/// </summary>
/// <param name="state"></param>
#define WhetherToDisplayTheCorrespondingSourceCodeLocation false

namespace ProjectDependenciesStd {


    void DisplaySourceCodeLocation(const char* file, int line) {
        if (WhetherToDisplayTheCorrespondingSourceCodeLocation) {
            std::cout << "位置：\t" << file << "\t" << line << std::endl;
        }
    }

    /// <summary>
    /// 打印提示
    /// </summary>
    /// <param name="context"></param>
    /// <param name="locationInformationRequired"></param>
    /// <param name="file"></param>
    /// <param name="line"></param>
    void DisplayPromptOrReason(const std::string& context, bool locationInformationRequired, const char* file, int line) {
        if (WhetherToDisplayPromptOrReason) {
            //std::cout << std::endl << "提示 开始：" << std::endl;
            std::cout << context << std::endl;
            if (locationInformationRequired) {
                DisplaySourceCodeLocation(file, line);
            }
            //std::cout << "提示 结束." << std::endl;
        }
    }


    void RunExe(const std::string& fileName, const std::string& path) {
        RunExeStd::RunExe(fileName, path);
    }

    bool RunExe2(const std::string& fileName, const std::string& dirName) {
        return RunExeStd::RunExe2(fileName, dirName);
    }

}