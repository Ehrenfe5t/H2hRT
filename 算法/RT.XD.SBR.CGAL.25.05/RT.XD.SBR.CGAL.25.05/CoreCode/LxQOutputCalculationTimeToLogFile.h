#pragma once

#include<string>

namespace OutputCalculationTimeToLogFileStd {


    /// <summary>
    /// 输出计算时长到日志文件中
    /// </summary>
    /// <param name="endTime">程序结束时刻</param>
    /// <param name="timeAll">程序运行的时长</param>
    void OutputCalculationTimeToLogFile(double endTime, double timeAll,
        std::string& osInformation, std::string& osOutPutLogInformationFileName);
}