

#include"LxQOutputCalculationTimeToLogFile.h"
#include"QzQFileBase.h"
#include<iostream>

namespace OutputCalculationTimeToLogFileStd {


    /// <summary>
    /// 输出计算时长到日志文件中
    /// </summary>
    /// <param name="endTime">程序结束时刻</param>
    /// <param name="timeAll">程序运行的时长</param>
    void OutputCalculationTimeToLogFile(double endTime, double timeAll,
        std::string& osInformation, std::string& osOutPutLogInformationFileName) {
        const char* logPath = osOutPutLogInformationFileName.c_str();
        if (FileOperateStd::ExistFile(logPath)) {
            //std::cout << logPath << ",已经存在。" << std::endl;
            FileOperateStd::RemoveFile(logPath);
            FileOperateStd::CreateNewFile(logPath);
        }
        else
        {
            FileOperateStd::CreateNewFile(logPath);
        }
        std::ofstream fileOpen; //定义ofstream 对象
        //fileOpen.open(logPath, std::ofstream::app);//追加
        fileOpen.open(logPath);
        if (!fileOpen)
        {
            std::cout << "没有打开文件！" << std::endl;
            fileOpen.close();
            return;
        }
        //fileOpen << "The total time taken for this execution is  " << timeAll << " ms." << std::endl;
        fileOpen << osInformation;
        fileOpen.close();
    }
}