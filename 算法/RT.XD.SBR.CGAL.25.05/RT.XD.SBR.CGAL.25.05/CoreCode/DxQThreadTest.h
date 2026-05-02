#pragma once

#include<iostream>
#include<thread>
#include<future>
#include<vector>
#include<queue>
#include<functional>
#include<memory>

#include"ThreadPool.h"

namespace ThreadTestStd {

    struct Teststruct {
        int start;
        int end;
        std::vector<double> sum;
        std::vector<bool> run_state;
        Teststruct(int start, int end) {
            this->start = start;
            this->end = end;
            sum.emplace_back(0);
            run_state.emplace_back(false);
        }
    };

    int fun1Core(
        Teststruct& obj) {

        for (int loop = obj.start; loop < obj.end; ++loop) {
            obj.sum[0] += loop*0.111118;
        }
        obj.run_state[0] = true;
        return (int)obj.sum[0];
    }



    void demo01() {
        ThreadPoolStd::ThreadPool threadPool;
        for (int i = 0;i<1e4;++i) {
            Teststruct obj(15+i, 11112100 + i);
            auto fut = threadPool.submit(fun1Core, obj);
        }
        
        threadPool.join();
    }

}


