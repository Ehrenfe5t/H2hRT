#pragma once

#include<time.h>

namespace CalRunTimeStd {

	class CalRunTime {
	public:
        CalRunTime(bool switchOfPrint);
        ~CalRunTime();

        double GetTimeAll() const;
    private:

        clock_t startTime;
        bool switchOfPrint;
	};

}