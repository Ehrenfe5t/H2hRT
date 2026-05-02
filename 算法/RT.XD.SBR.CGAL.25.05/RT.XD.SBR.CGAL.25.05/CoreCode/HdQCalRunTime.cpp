
#include"HdQCalRunTime.h"
#include"LxQProjectDependencies.h"
#include<iostream>
#include<sstream>
namespace CalRunTimeStd {


    CalRunTime::CalRunTime(bool switchOfPrint) {
        this->startTime = clock();
        this->switchOfPrint = switchOfPrint;
    }

    CalRunTime::~CalRunTime() {
        if (this->switchOfPrint) {
            clock_t endTime = clock();
            double timeAll = (double)((double)endTime - (double)this->startTime);
            {
                std::ostringstream oss;
                oss << "CalRunTime运行时间为：" << timeAll << " ms.";
                ProjectDependenciesStd::DisplayPromptOrReason(oss.str(), false, __FILE__, __LINE__);
            }
        }
    }

    double CalRunTime::GetTimeAll() const
    {
        clock_t endTime = clock();
        double timeAll = (double)((double)endTime - (double)this->startTime);
        return timeAll;
    }

}