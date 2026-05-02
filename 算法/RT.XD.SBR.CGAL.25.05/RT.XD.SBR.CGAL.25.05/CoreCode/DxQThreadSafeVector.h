#pragma once
#include <vector>
#include <mutex>

namespace ThreadSafeVectorStd {

    template<typename T>
    class ThreadSafeVector {
    private:
        std::vector<T> vec;
        std::mutex mtx;

    public:
        // 添加元素
        void emplace_back(const T& value) {
            std::lock_guard<std::mutex> lock(mtx);
            vec.emplace_back(value);
        }

        // 获取元素个数
        size_t size() const {
            std::lock_guard<std::mutex> lock(mtx);
            return vec.size();
        }

        // 获取指定位置的元素
        T& operator[](size_t index) {
            std::lock_guard<std::mutex> lock(mtx);
            return vec[index];
        }

        // 其他必要的成员函数可以以同样的方式实现锁定
        // ...
    };

}